#!/usr/bin/env node

import { createConnection, TextDocuments, ProposedFeatures, InitializeParams, DidChangeConfigurationNotification, CompletionItem, CompletionItemKind, TextDocumentPositionParams, TextDocumentSyncKind, InitializeResult } from 'vscode-languageserver/node.js';
import { TextDocument } from 'vscode-languageserver-textdocument';
import * as ts from 'typescript';
import { URI } from 'vscode-uri';

// Create a connection for the server
const connection = createConnection(ProposedFeatures.all);

// Create a simple text document manager
const documents = new TextDocuments(TextDocument);

let hasConfigurationCapability = false;
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;

// TypeScript language service
let languageService;
let compilerOptions = {
    target: ts.ScriptTarget.ES2020,
    module: ts.ModuleKind.ESNext,
    moduleResolution: ts.ModuleResolutionKind.NodeJs,
    allowJs: true,
    checkJs: false,
    jsx: ts.JsxEmit.ReactJSX,
    declaration: true,
    outDir: './dist',
    strict: true,
    esModuleInterop: true,
    skipLibCheck: true,
    forceConsistentCasingInFileNames: true,
    resolveJsonModule: true
};

connection.onInitialize((params) => {
    const capabilities = params.capabilities;

    hasConfigurationCapability = !!(capabilities.workspace && !!capabilities.workspace.configuration);
    hasWorkspaceFolderCapability = !!(capabilities.workspace && !!capabilities.workspace.workspaceFolders);
    hasDiagnosticRelatedInformationCapability = !!(capabilities.textDocument && capabilities.textDocument.publishDiagnostics && capabilities.textDocument.publishDiagnostics.relatedInformation);

    const result = {
        capabilities: {
            textDocumentSync: TextDocumentSyncKind.Incremental,
            completionProvider: {
                resolveProvider: true,
                triggerCharacters: ['.', ':', '<', '"', "'", '/', '@']
            },
            hoverProvider: true,
            signatureHelpProvider: {
                triggerCharacters: ['(', ',']
            },
            definitionProvider: true,
            referencesProvider: true,
            documentHighlightProvider: true,
            documentSymbolProvider: true,
            workspaceSymbolProvider: true,
            codeActionProvider: true,
            codeLensProvider: {
                resolveProvider: false
            },
            documentFormattingProvider: true,
            documentRangeFormattingProvider: true,
            renameProvider: {
                prepareProvider: true
            },
            foldingRangeProvider: true
        }
    };

    if (hasWorkspaceFolderCapability) {
        result.capabilities.workspace = {
            workspaceFolders: {
                supported: true
            }
        };
    }

    return result;
});

connection.onInitialized(() => {
    if (hasConfigurationCapability) {
        connection.client.register(DidChangeConfigurationNotification.type, undefined);
    }
    if (hasWorkspaceFolderCapability) {
        connection.workspace.onDidChangeWorkspaceFolders(_event => {
            connection.console.log('Workspace folder change event received.');
        });
    }

    // Initialize TypeScript language service
    initializeLanguageService();
});

function initializeLanguageService() {
    const servicesHost = {
        getScriptFileNames: () => {
            return Array.from(documents.keys());
        },
        getScriptVersion: (fileName) => {
            const document = documents.get(fileName);
            return document ? document.version.toString() : '0';
        },
        getScriptSnapshot: (fileName) => {
            const document = documents.get(fileName);
            if (document) {
                return ts.ScriptSnapshot.fromString(document.getText());
            }
            return undefined;
        },
        getCurrentDirectory: () => process.cwd(),
        getCompilationSettings: () => compilerOptions,
        getDefaultLibFileName: (options) => ts.getDefaultLibFilePath(options),
        fileExists: ts.sys.fileExists,
        readFile: ts.sys.readFile,
        readDirectory: ts.sys.readDirectory,
        directoryExists: ts.sys.directoryExists,
        getDirectories: ts.sys.getDirectories
    };

    languageService = ts.createLanguageService(servicesHost, ts.createDocumentRegistry());
}

// Completion provider
connection.onCompletion((textDocumentPosition) => {
    const document = documents.get(textDocumentPosition.textDocument.uri);
    if (!document || !languageService) {
        return [];
    }

    const fileName = URI.parse(textDocumentPosition.textDocument.uri).fsPath;
    const position = document.offsetAt(textDocumentPosition.position);

    const completions = languageService.getCompletionsAtPosition(
        fileName,
        position,
        undefined
    );

    if (!completions) {
        return [];
    }

    return completions.entries.map(entry => {
        const item = {
            label: entry.name,
            kind: getCompletionItemKind(entry.kind),
            data: entry
        };
        return item;
    });
});

// Completion resolve
connection.onCompletionResolve((item) => {
    if (item.data && languageService) {
        const details = languageService.getCompletionEntryDetails(
            item.data.fileName || '',
            item.data.position || 0,
            item.data.name,
            undefined,
            undefined,
            undefined,
            undefined
        );
        
        if (details) {
            item.detail = ts.displayPartsToString(details.displayParts);
            item.documentation = ts.displayPartsToString(details.documentation);
        }
    }
    return item;
});

// Hover provider
connection.onHover((params) => {
    const document = documents.get(params.textDocument.uri);
    if (!document || !languageService) {
        return null;
    }

    const fileName = URI.parse(params.textDocument.uri).fsPath;
    const position = document.offsetAt(params.position);

    const info = languageService.getQuickInfoAtPosition(fileName, position);
    if (!info) {
        return null;
    }

    return {
        contents: {
            kind: 'markdown',
            value: ts.displayPartsToString(info.displayParts)
        },
        range: {
            start: document.positionAt(info.textSpan.start),
            end: document.positionAt(info.textSpan.start + info.textSpan.length)
        }
    };
});

function getCompletionItemKind(kind) {
    switch (kind) {
        case ts.ScriptElementKind.primitiveType:
        case ts.ScriptElementKind.keyword:
            return CompletionItemKind.Keyword;
        case ts.ScriptElementKind.constElement:
            return CompletionItemKind.Constant;
        case ts.ScriptElementKind.letElement:
        case ts.ScriptElementKind.variableElement:
        case ts.ScriptElementKind.localVariableElement:
        case ts.ScriptElementKind.alias:
            return CompletionItemKind.Variable;
        case ts.ScriptElementKind.memberVariableElement:
        case ts.ScriptElementKind.memberGetAccessorElement:
        case ts.ScriptElementKind.memberSetAccessorElement:
            return CompletionItemKind.Field;
        case ts.ScriptElementKind.functionElement:
        case ts.ScriptElementKind.memberFunctionElement:
        case ts.ScriptElementKind.constructSignatureElement:
        case ts.ScriptElementKind.callSignatureElement:
        case ts.ScriptElementKind.indexSignatureElement:
            return CompletionItemKind.Function;
        case ts.ScriptElementKind.enumElement:
            return CompletionItemKind.Enum;
        case ts.ScriptElementKind.moduleElement:
        case ts.ScriptElementKind.externalModuleName:
            return CompletionItemKind.Module;
        case ts.ScriptElementKind.classElement:
        case ts.ScriptElementKind.typeElement:
            return CompletionItemKind.Class;
        case ts.ScriptElementKind.interfaceElement:
            return CompletionItemKind.Interface;
        case ts.ScriptElementKind.warning:
            return CompletionItemKind.Text;
        case ts.ScriptElementKind.scriptElement:
            return CompletionItemKind.File;
        case ts.ScriptElementKind.directory:
            return CompletionItemKind.Folder;
        case ts.ScriptElementKind.string:
            return CompletionItemKind.Constant;
        default:
            return CompletionItemKind.Property;
    }
}

// Document change events
documents.onDidChangeContent(change => {
    validateTextDocument(change.document);
});

async function validateTextDocument(textDocument) {
    if (!languageService) {
        return;
    }

    const fileName = URI.parse(textDocument.uri).fsPath;
    const diagnostics = [];

    try {
        const syntacticDiagnostics = languageService.getSyntacticDiagnostics(fileName);
        const semanticDiagnostics = languageService.getSemanticDiagnostics(fileName);
        
        [...syntacticDiagnostics, ...semanticDiagnostics].forEach(diagnostic => {
            const start = textDocument.positionAt(diagnostic.start || 0);
            const end = textDocument.positionAt((diagnostic.start || 0) + (diagnostic.length || 0));
            
            diagnostics.push({
                severity: diagnostic.category === ts.DiagnosticCategory.Error ? 1 : 2,
                range: { start, end },
                message: ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n'),
                source: 'typescript'
            });
        });
    } catch (error) {
        connection.console.error(`Error validating ${fileName}: ${error.message}`);
    }

    connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
}

// Make the text document manager listen on the connection
documents.listen(connection);

// Listen on the connection
connection.listen();

connection.console.log('TypeScript Language Server started');