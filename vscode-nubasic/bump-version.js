const fs = require('fs');
const path = require('path');

const root = __dirname;
const packageJsonPath = path.join(root, 'package.json');
const packageLockPath = path.join(root, 'package-lock.json');

function readJson(filePath) {
  return JSON.parse(fs.readFileSync(filePath, 'utf8').replace(/^\uFEFF/, ''));
}

const pkg = readJson(packageJsonPath);
const match = /^(\d+)\.(\d+)\.(\d+)$/.exec(pkg.version);
if (!match) {
  throw new Error(`Unsupported VS Code extension version '${pkg.version}'. Expected MAJOR.MINOR.PATCH.`);
}

pkg.version = `${match[1]}.${match[2]}.${Number(match[3]) + 1}`;
fs.writeFileSync(packageJsonPath, JSON.stringify(pkg, null, 2) + '\n');

if (fs.existsSync(packageLockPath)) {
  const lock = readJson(packageLockPath);
  lock.version = pkg.version;
  if (lock.packages && lock.packages['']) {
    lock.packages[''].version = pkg.version;
  }
  fs.writeFileSync(packageLockPath, JSON.stringify(lock, null, 2) + '\n');
}

console.log(`VS Code extension version bumped to ${pkg.version}`);
