const vscode = require('vscode');
const { spawn } = require('child_process');

function activate(context) {
  const disposable = vscode.commands.registerCommand('extension.scanForErrors', () => {
    const editor = vscode.window.activeTextEditor;
    if (editor) {
      const filePath = editor.document.fileName;

      const eslint = spawn('npx', ['eslint', '--fix', filePath]);

      eslint.stdout.on('data', (data) => {
        console.log(data.toString());
      });

      eslint.stderr.on('data', (data) => {
        console.error(data.toString());
      });

      eslint.on('exit', (code) => {
        if (code === 0) {
          vscode.window.showInformationMessage('No errors found.');
        } else {
          vscode.window.showErrorMessage('Errors found. Check the output for details.');
        }
      });
    }
  });

  context.subscriptions.push(disposable);
}

module.exports = {
  activate,
};
