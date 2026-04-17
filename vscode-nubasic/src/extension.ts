import * as vscode from 'vscode';
import * as path from 'path';
import * as cp from 'child_process';

let terminal: vscode.Terminal | undefined;

export function activate(context: vscode.ExtensionContext) {
    context.subscriptions.push(
        vscode.commands.registerCommand('nubasic.run', runFile),
        vscode.commands.registerCommand('nubasic.stop', stopFile),
        vscode.window.onDidCloseTerminal(t => {
            if (t === terminal) terminal = undefined;
        })
    );
}

async function resolveExecutable(): Promise<string> {
    const config = vscode.workspace.getConfiguration('nubasic');
    const configured = config.get<string>('executablePath', '').trim();
    if (configured && configured !== 'nubasic') {
        return configured;
    }

    // Try registry: HKLM\Software\nuBASIC\InstallDir -> <dir>\bin\nubasic.exe
    const fromReg = await readRegistryInstallDir();
    if (fromReg) {
        return path.join(fromReg, 'bin', 'nubasic.exe');
    }

    return 'nubasic';
}

function readRegistryInstallDir(): Promise<string | undefined> {
    return new Promise(resolve => {
        const keys = [
            'HKLM\\Software\\nuBASIC',
            'HKLM\\Software\\WOW6432Node\\nuBASIC',
            'HKCU\\Software\\nuBASIC',
        ];

        let remaining = keys.length;
        let found = false;

        for (const key of keys) {
            cp.exec(`reg query "${key}" /v InstallDir`, (err, stdout) => {
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

async function runFile() {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showErrorMessage('No active file to run.');
        return;
    }
    if (editor.document.languageId !== 'nubasic') {
        vscode.window.showErrorMessage('Active file is not a nuBASIC file.');
        return;
    }

    if (editor.document.isDirty) {
        await editor.document.save();
    }

    const exe = await resolveExecutable();
    const filePath = editor.document.fileName;

    getOrCreateTerminal();
    terminal!.show(true);
    terminal!.sendText(`& "${exe}" "${filePath}"`);
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

export function deactivate() {
    terminal?.dispose();
}
