import * as vscode from 'vscode';
import * as path from 'path';
import * as cp from 'child_process';

let terminal: vscode.Terminal | undefined;
let debugLog: vscode.OutputChannel | undefined;

type DebugProtocolMessage = {
    seq: number;
    type: string;
    [key: string]: unknown;
};

type DebugRequest = DebugProtocolMessage & {
    command: string;
    arguments?: Record<string, unknown>;
};

type LaunchArguments = {
    program: string;
    executablePath?: string;
    cwd?: string;
    stopOnEntry?: boolean;
};

type PendingCommand = {
    resolve: (output: string) => void;
    reject: (err: Error) => void;
    output: string;
};

export function activate(context: vscode.ExtensionContext) {
    const debugProvider = new NuBasicDebugConfigurationProvider();
    debugLog = vscode.window.createOutputChannel('nuBASIC Debug');

    context.subscriptions.push(
        vscode.commands.registerCommand('nubasic.debug', debugFile),
        vscode.commands.registerCommand('nubasic.run', runFile),
        vscode.commands.registerCommand('nubasic.stop', stopFile),
        vscode.debug.registerDebugConfigurationProvider('nubasic', debugProvider),
        vscode.debug.registerDebugAdapterDescriptorFactory(
            'nubasic',
            new NuBasicDebugAdapterFactory()
        ),
        debugLog,
        vscode.window.onDidCloseTerminal(t => {
            if (t === terminal) terminal = undefined;
        })
    );
}

async function resolveExecutable(override?: string): Promise<string> {
    const configuredOverride = override?.trim();
    if (configuredOverride) {
        return configuredOverride;
    }

    const config = vscode.workspace.getConfiguration('nubasic');
    const configured = config.get<string>('executablePath', '').trim();
    if (configured && configured !== 'nubasic') {
        return configured;
    }

    const fromReg = await readRegistryInstallDir();
    if (fromReg) {
        return path.join(fromReg, 'bin', 'nubasic.exe');
    }

    return 'nubasic';
}

function readRegistryInstallDir(): Promise<string | undefined> {
    if (process.platform !== 'win32') {
        return Promise.resolve(undefined);
    }

    return new Promise(resolve => {
        const keys = [
            'HKLM\\Software\\nuBASIC',
            'HKLM\\Software\\WOW6432Node\\nuBASIC',
            'HKCU\\Software\\nuBASIC',
        ];

        let remaining = keys.length;
        let found = false;

        for (const key of keys) {
            cp.exec(`reg query "${key}" /v InstallDir`, (err: Error | null, stdout: string) => {
                if (!found && !err && stdout) {
                    const match = stdout.match(/InstallDir\s+REG_SZ\s+(.+)/);
                    if (match) {
                        found = true;
                        resolve(match[1].trim());
                        return;
                    }
                }
                if (--remaining === 0 && !found) {
                    resolve(undefined);
                }
            });
        }
    });
}

async function debugFile() {
    const editor = vscode.window.activeTextEditor;
    if (!await ensureNuBasicDocument(editor, 'debug')) {
        return;
    }

    if (editor!.document.isDirty) {
        await editor!.document.save();
    }

    if (documentUsesGraphics(editor!.document)) {
        vscode.window.showInformationMessage(
            'This nuBASIC program uses graphics; launching it in the GDI console.'
        );
        await runFile();
        return;
    }

    const folder = vscode.workspace.getWorkspaceFolder(editor!.document.uri);
    const config: vscode.DebugConfiguration = {
        type: 'nubasic',
        request: 'launch',
        name: 'Debug nuBASIC File',
        program: editor!.document.fileName,
    };

    await vscode.debug.startDebugging(folder, config);
}

async function runFile() {
    const editor = vscode.window.activeTextEditor;
    if (!await ensureNuBasicDocument(editor, 'run')) {
        return;
    }

    if (editor!.document.isDirty) {
        await editor!.document.save();
    }

    const exe = await resolveExecutable();
    const filePath = editor!.document.fileName.replace(/\\/g, '/');

    getOrCreateTerminal();
    terminal!.show(true);
    const cmd = process.platform === 'win32'
        ? `& "${exe}" -e "${filePath}"`
        : `"${exe}" -e "${filePath}"`;
    terminal!.sendText(cmd);
}

async function ensureNuBasicDocument(
    editor: vscode.TextEditor | undefined,
    action: string
): Promise<boolean> {
    if (!editor) {
        vscode.window.showErrorMessage(`No active file to ${action}.`);
        return false;
    }
    if (editor.document.languageId !== 'nubasic') {
        vscode.window.showErrorMessage('Active file is not a nuBASIC file.');
        return false;
    }
    return true;
}

function stopFile() {
    terminal?.sendText('\x03');
}

function getOrCreateTerminal() {
    if (!terminal || terminal.exitStatus !== undefined) {
        terminal = vscode.window.createTerminal({
            name: 'nuBASIC',
            cwd: workspaceRoot()
        });
    }
}

function workspaceRoot(): string {
    const editor = vscode.window.activeTextEditor;
    if (editor) {
        const ws = vscode.workspace.getWorkspaceFolder(editor.document.uri);
        if (ws) return ws.uri.fsPath;
        return path.dirname(editor.document.fileName);
    }
    return process.cwd();
}

function documentUsesGraphics(document: vscode.TextDocument): boolean {
    const graphicsKeywords = [
        'ellipse',
        'fillellipse',
        'fillrect',
        'getpixel',
        'line',
        'movewindow',
        'plotimage',
        'rect',
        'refresh',
        'screen',
        'screenlock',
        'screenunlock',
        'setpixel',
        'textout',
    ];

    return document
        .getText()
        .split(/\r?\n/)
        .some(line => sourceLineUsesAnyKeyword(line, graphicsKeywords));
}

function sourceLineUsesAnyKeyword(line: string, keywords: string[]): boolean {
    const withoutComment = stripBasicComment(line).toLowerCase();
    if (!withoutComment.trim()) {
        return false;
    }

    const withoutLineNumber = withoutComment.replace(/^\s*\d+\s+/, '');
    if (/^\s*rem(?:\s|$)/.test(withoutLineNumber)) {
        return false;
    }

    return keywords.some(keyword => {
        const escaped = keyword.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
        return new RegExp(`(^|[^a-z0-9_%$])${escaped}([^a-z0-9_%$]|$)`).test(
            withoutLineNumber
        );
    });
}

function stripBasicComment(line: string): string {
    let inString = false;
    for (let i = 0; i < line.length; ++i) {
        const ch = line[i];
        if (ch === '"') {
            inString = !inString;
        } else if (ch === '\'' && !inString) {
            return line.slice(0, i);
        }
    }
    return line;
}

class NuBasicDebugConfigurationProvider implements vscode.DebugConfigurationProvider {
    resolveDebugConfiguration(
        folder: vscode.WorkspaceFolder | undefined,
        config: vscode.DebugConfiguration
    ): vscode.ProviderResult<vscode.DebugConfiguration> {
        if (!config.type && !config.request && !config.name) {
            config.type = 'nubasic';
            config.request = 'launch';
            config.name = 'Debug nuBASIC File';
            config.program = '${file}';
        }

        if (!config.program) {
            const editor = vscode.window.activeTextEditor;
            if (editor?.document.languageId === 'nubasic') {
                config.program = editor.document.fileName;
            }
        }

        if (!config.cwd
            && typeof config.program === 'string'
            && !config.program.includes('${')) {
            config.cwd = path.dirname(config.program);
        }

        return config;
    }
}

class NuBasicDebugAdapterFactory implements vscode.DebugAdapterDescriptorFactory {
    createDebugAdapterDescriptor(): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
        return new vscode.DebugAdapterInlineImplementation(new NuBasicDebugSession());
    }
}

class NuBasicDebugSession implements vscode.DebugAdapter {
    private readonly emitter = new vscode.EventEmitter<DebugProtocolMessage>();
    readonly onDidSendMessage = this.emitter.event;

    private seq = 1;
    private process: cp.ChildProcessWithoutNullStreams | undefined;
    private pending: PendingCommand | undefined;
    private breakpoints = new Map<string, number[]>();
    private launchArgs: LaunchArguments | undefined;
    private currentLine = 0;
    private programPath = '';
    private variablesReference = 1;
    private lastVariables = '';
    private stepModeEnabled = false;
    private pendingTimer: NodeJS.Timeout | undefined;

    handleMessage(message: DebugProtocolMessage): void {
        const request = message as DebugRequest;

        switch (request.command) {
            case 'initialize':
                this.respond(request, {
                    supportsConfigurationDoneRequest: true,
                    supportsTerminateRequest: true,
                    supportsRestartRequest: false,
                    supportsStepBack: false,
                    supportsEvaluateForHovers: false,
                });
                this.event('initialized');
                break;

            case 'launch':
                this.launchArgs = request.arguments as LaunchArguments;
                this.programPath = this.launchArgs.program;
                this.respond(request);
                break;

            case 'setBreakpoints':
                this.setBreakpoints(request);
                break;

            case 'configurationDone':
                this.respond(request);
                this.startDebugging();
                break;

            case 'threads':
                this.respond(request, { threads: [{ id: 1, name: 'nuBASIC' }] });
                break;

            case 'stackTrace':
                this.respond(request, this.stackTraceBody());
                break;

            case 'scopes':
                this.respond(request, {
                    scopes: [{
                        name: 'nuBASIC Runtime',
                        variablesReference: this.variablesReference,
                        expensive: false,
                    }]
                });
                break;

            case 'variables':
                this.readVariables(request);
                break;

            case 'continue':
                this.respond(request, { allThreadsContinued: true });
                this.continueExecution(false);
                break;

            case 'next':
            case 'stepIn':
            case 'stepOut':
                this.respond(request);
                this.continueExecution(true);
                break;

            case 'pause':
                this.respond(request);
                this.event('stopped', { reason: 'pause', threadId: 1 });
                break;

            case 'disconnect':
            case 'terminate':
                this.respond(request);
                this.shutdown();
                this.event('terminated');
                break;

            default:
                this.respond(request);
                break;
        }
    }

    dispose(): void {
        this.shutdown();
        this.emitter.dispose();
    }

    private setBreakpoints(request: DebugRequest) {
        const args = request.arguments ?? {};
        const source = args.source as { path?: string } | undefined;
        const sourcePath = source?.path ?? this.programPath;
        const requested = args.breakpoints as Array<{ line: number }> | undefined;
        const lines = (requested ?? []).map(bp => bp.line).filter(line => line > 0);

        this.breakpoints.set(sourcePath, lines);
        this.respond(request, {
            breakpoints: lines.map(line => ({ verified: true, line }))
        });
    }

    private async startDebugging() {
        if (!this.launchArgs) {
            this.event('terminated');
            return;
        }

        try {
            const exe = await resolveExecutable(this.launchArgs.executablePath);
            const cwd = this.launchArgs.cwd || path.dirname(this.launchArgs.program);
            this.log(`launch exe="${exe}" cwd="${cwd}" program="${this.launchArgs.program}"`);
            this.process = cp.spawn(exe, ['-t'], { cwd, windowsHide: true });
            this.process.stdout.on('data', (chunk: Buffer) => this.onOutput(chunk.toString()));
            this.process.stderr.on('data', (chunk: Buffer) => this.output(chunk.toString(), 'stderr'));
            this.process.on('exit', (code: number | null, signal: NodeJS.Signals | null) => {
                if (this.pending) {
                    const pending = this.pending;
                    this.clearPendingState();
                    const reason = signal
                        ? `signal ${signal}`
                        : `exit code ${code ?? 0}`;
                    this.log(`process exit before response (${reason})`);
                    pending.reject(new Error(`nuBASIC exited before responding (${reason}).`));
                }
                this.event('terminated');
            });
            this.process.on('error', (err: Error) => {
                if (this.pending) {
                    const pending = this.pending;
                    this.clearPendingState();
                    pending.reject(err);
                }
                this.output(`Unable to start nuBASIC: ${err.message}\n`, 'stderr');
                this.event('terminated');
            });

            await this.waitForPrompt();
            await this.sendCommand(`load "${path.basename(this.launchArgs.program)}"`);
            await this.sendCommand('clrbrk');

            for (const line of this.breakpointsForProgram()) {
                await this.sendCommand(`break ${line}`);
            }

            if (this.launchArgs.stopOnEntry && this.breakpointsForProgram().length === 0) {
                await this.sendCommand('ston');
                this.stepModeEnabled = true;
            }

            const output = await this.sendCommand('run');
            const stoppedLine = this.parseStoppedLine(output)
                || (this.launchArgs.stopOnEntry
                    ? await this.refreshCurrentLine(true)
                    : await this.refreshCurrentLine(true, true));
            if (stoppedLine > 0) {
                this.currentLine = stoppedLine;
                this.event('stopped', {
                    reason: this.launchArgs.stopOnEntry ? 'entry' : 'breakpoint',
                    threadId: 1,
                });
            } else {
                this.event('terminated');
                this.shutdown();
            }
        } catch (err) {
            const message = err instanceof Error ? err.message : String(err);
            this.output(`nuBASIC debug session failed: ${message}\n`, 'stderr');
            this.event('terminated');
            this.shutdown();
        }
    }

    private async continueExecution(step: boolean) {
        try {
            if (!step && this.stepModeEnabled) {
                await this.sendCommand('stoff');
                this.stepModeEnabled = false;
            }

            if (step) {
                await this.sendCommand('ston');
                this.stepModeEnabled = true;
            }

            const output = await this.sendCommand('cont');
            if (step) {
                await this.sendCommand('stoff');
                this.stepModeEnabled = false;
            }

            const stoppedLine = this.parseStoppedLine(output)
                || (step
                    ? await this.refreshCurrentLine(true)
                    : await this.refreshCurrentLine(true, true));
            if (stoppedLine > 0) {
                this.currentLine = stoppedLine;
                this.event('stopped', {
                    reason: step ? 'step' : 'breakpoint',
                    threadId: 1,
                });
            } else {
                this.event('terminated');
                this.shutdown();
            }
        } catch (err) {
            const message = err instanceof Error ? err.message : String(err);
            this.output(`nuBASIC debug command failed: ${message}\n`, 'stderr');
            this.event('terminated');
            this.shutdown();
        }
    }

    private async readVariables(request: DebugRequest) {
        try {
            this.lastVariables = await this.sendCommand('vars');
            this.respond(request, {
                variables: this.parseVariables(this.lastVariables)
            });
        } catch {
            this.respond(request, { variables: [] });
        }
    }

    private sendCommand(command: string): Promise<string> {
        if (!this.process || this.process.killed || !this.process.stdin.writable) {
            return Promise.reject(new Error('nuBASIC is not running.'));
        }
        if (this.pending) {
            return Promise.reject(new Error('A nuBASIC debug command is already running.'));
        }

        return new Promise((resolve, reject) => {
            this.pending = { resolve, reject, output: '' };
            this.log(`>>> ${command}`);
            this.process!.stdin.write(`${command}\n`);
        });
    }

    private waitForPrompt(): Promise<string> {
        if (!this.process) {
            return Promise.reject(new Error('nuBASIC is not running.'));
        }
        return new Promise((resolve, reject) => {
            this.pending = { resolve, reject, output: '' };
            this.pendingTimer = setTimeout(() => {
                if (!this.pending) {
                    return;
                }

                const pending = this.pending;
                this.clearPendingState();
                const details = pending.output.trim();
                this.log('timeout waiting for prompt');
                reject(new Error(details
                    ? `Timed out waiting for the nuBASIC prompt. Output so far:\n${details}`
                    : 'Timed out waiting for the nuBASIC prompt.'));
            }, 5000);
        });
    }

    private onOutput(text: string) {
        this.log(text.replace(/\r/g, '\\r').replace(/\n/g, '\\n\n'));
        this.output(text, 'stdout');

        if (!this.pending) {
            return;
        }

        this.pending.output += text;
        if (this.hasCommandCompleted(this.pending.output)) {
            const pending = this.pending;
            this.clearPendingState();
            pending.resolve(pending.output);
        }
    }

    private clearPendingState() {
        if (this.pendingTimer) {
            clearTimeout(this.pendingTimer);
            this.pendingTimer = undefined;
        }
        this.pending = undefined;
    }

    private log(message: string) {
        debugLog?.appendLine(message);
    }

    private hasCommandCompleted(output: string): boolean {
        return /(^|\r?\n)Ok\.\r?\n/.test(output)
            || /(^|\r?\n)Ready\.\r?\n/.test(output)
            || /Type 'cont' to continue\r?\n/.test(output)
            || /Runtime Error #\d+ at \d+/.test(output)
            || /At line \d+:/.test(output)
            || /Error loading file or file not found/.test(output)
            || /Syntax Error/.test(output);
    }

    private async refreshCurrentLine(
        allowSourceListing: boolean,
        onlyIfBreakpoint = false
    ): Promise<number> {
        const output = await this.sendCommand('vars');
        this.lastVariables = output;
        const match = output.match(/Current line\s*:\s*(\d+)/i);
        if (match) {
            return Number(match[1]);
        }

        if (allowSourceListing) {
            const listing = output.match(/(?:^|\r?\n)\s*(\d+)\s+\S/);
            if (listing) {
                const line = Number(listing[1]);
                if (!onlyIfBreakpoint || this.breakpointsForProgram().includes(line)) {
                    return line;
                }
            }
        }

        return 0;
    }

    private parseStoppedLine(output: string): number {
        const match = output.match(/Execution stopped at (?:breakpoint|STOP instruction), line (\d+)/i);
        return match ? Number(match[1]) : 0;
    }

    private parseVariables(output: string): Array<{ name: string; value: string; variablesReference: number }> {
        const variables: Array<{ name: string; value: string; variablesReference: number }> = [];
        const lines = output
            .split(/\r?\n/)
            .map(line => line.trim())
            .filter(line => line && line !== 'Ok.' && !line.startsWith('Ready.'));

        for (const line of lines) {
            const match = line.match(/^([^:]+):\s*(.*)$/);
            if (match) {
                variables.push({
                    name: match[1].trim(),
                    value: match[2].trim(),
                    variablesReference: 0,
                });
            }
        }

        if (variables.length === 0 && output.trim()) {
            variables.push({
                name: 'vars',
                value: output.trim(),
                variablesReference: 0,
            });
        }

        return variables;
    }

    private stackTraceBody() {
        const line = this.currentLine > 0 ? this.currentLine : 1;
        return {
            stackFrames: [{
                id: 1,
                name: 'nuBASIC program',
                source: {
                    name: path.basename(this.programPath),
                    path: this.programPath,
                },
                line,
                column: 1,
            }],
            totalFrames: 1,
        };
    }

    private breakpointsForProgram(): number[] {
        const direct = this.breakpoints.get(this.programPath);
        if (direct) {
            return direct;
        }

        const canonical = path.resolve(this.programPath).toLowerCase();
        for (const [source, lines] of this.breakpoints) {
            if (path.resolve(source).toLowerCase() === canonical) {
                return lines;
            }
        }

        return [];
    }

    private output(output: string, category: string) {
        this.event('output', { category, output });
    }

    private respond(request: DebugRequest, body?: unknown) {
        this.emitter.fire({
            type: 'response',
            seq: this.seq++,
            request_seq: request.seq,
            success: true,
            command: request.command,
            body,
        });
    }

    private event(event: string, body?: unknown) {
        this.emitter.fire({
            type: 'event',
            seq: this.seq++,
            event,
            body,
        });
    }

    private shutdown() {
        if (this.pending) {
            const pending = this.pending;
            this.clearPendingState();
            pending.resolve(pending.output);
        }

        if (this.process && !this.process.killed) {
            this.process.kill();
        }
        this.process = undefined;
    }
}

export function deactivate() {
    terminal?.dispose();
}
