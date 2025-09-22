#include "lsp_wrapper.hpp"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Parse/ParseAST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/JSON.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace miko {
namespace lsp {

// Implementation class using PIMPL pattern
class LSPServer::Impl {
public:
    Impl() : initialized_(false), shutdown_(false) {}
    
    ~Impl() {
        shutdown();
    }
    
    bool initialize(const std::string& rootPath, const LSPCapabilities& capabilities) {
        if (initialized_) return false;
        
        rootPath_ = rootPath;
        capabilities_ = capabilities;
        
        // Initialize clang tooling
        std::string errorMessage;
        compilationDatabase_ = clang::tooling::JSONCompilationDatabase::loadFromDirectory(
            rootPath, errorMessage);
        
        if (!compilationDatabase_) {
            // Create a simple compilation database if none exists
            std::vector<clang::tooling::CompileCommand> commands;
            commands.emplace_back(
                rootPath,
                "main.cpp",
                std::vector<std::string>{"-std=c++17", "-I" + rootPath},
                "main.o"
            );
            compilationDatabase_ = std::make_unique<clang::tooling::FixedCompilationDatabase>(
                rootPath, commands);
        }
        
        initialized_ = true;
        return true;
    }
    
    void shutdown() {
        if (!initialized_ || shutdown_) return;
        
        shutdown_ = true;
        documents_.clear();
        diagnostics_.clear();
        initialized_ = false;
    }
    
    bool didOpen(const std::string& uri, const std::string& languageId, const std::string& text) {
        if (!initialized_) return false;
        
        std::lock_guard<std::mutex> lock(documentsMutex_);
        documents_[uri] = text;
        
        // Run diagnostics in background
        std::thread([this, uri]() {
            runDiagnostics(uri);
        }).detach();
        
        return true;
    }
    
    bool didChange(const std::string& uri, const std::string& text) {
        if (!initialized_) return false;
        
        std::lock_guard<std::mutex> lock(documentsMutex_);
        documents_[uri] = text;
        
        // Run diagnostics in background
        std::thread([this, uri]() {
            runDiagnostics(uri);
        }).detach();
        
        return true;
    }
    
    bool didSave(const std::string& uri) {
        if (!initialized_) return false;
        
        // Run full analysis on save
        std::thread([this, uri]() {
            runDiagnostics(uri);
        }).detach();
        
        return true;
    }
    
    bool didClose(const std::string& uri) {
        if (!initialized_) return false;
        
        std::lock_guard<std::mutex> lock(documentsMutex_);
        documents_.erase(uri);
        diagnostics_.erase(uri);
        
        return true;
    }
    
    std::vector<CompletionItem> completion(const TextDocumentPositionParams& params) {
        std::vector<CompletionItem> items;
        
        if (!initialized_) return items;
        
        // Basic C/C++ keywords and common completions
        std::vector<std::string> keywords = {
            "auto", "break", "case", "char", "const", "continue", "default", "do",
            "double", "else", "enum", "extern", "float", "for", "goto", "if",
            "int", "long", "register", "return", "short", "signed", "sizeof", "static",
            "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
            "class", "namespace", "template", "typename", "public", "private", "protected",
            "virtual", "override", "final", "constexpr", "nullptr", "decltype"
        };
        
        for (const auto& keyword : keywords) {
            CompletionItem item;
            item.label = keyword;
            item.kind = 14; // Keyword
            item.insertText = keyword;
            items.push_back(item);
        }
        
        return items;
    }
    
    HoverResult hover(const TextDocumentPositionParams& params) {
        HoverResult result;
        
        if (!initialized_) return result;
        
        // Basic hover information
        result.contents = "C/C++ symbol information";
        result.range.start = params.position;
        result.range.end = params.position;
        result.range.end.character += 5; // Approximate word length
        
        return result;
    }
    
    std::vector<Range> definition(const TextDocumentPositionParams& params) {
        std::vector<Range> ranges;
        
        if (!initialized_) return ranges;
        
        // Return current position as placeholder
        Range range;
        range.start = params.position;
        range.end = params.position;
        ranges.push_back(range);
        
        return ranges;
    }
    
    std::vector<Range> references(const TextDocumentPositionParams& params) {
        std::vector<Range> ranges;
        
        if (!initialized_) return ranges;
        
        // Return current position as placeholder
        Range range;
        range.start = params.position;
        range.end = params.position;
        ranges.push_back(range);
        
        return ranges;
    }
    
    std::vector<Diagnostic> getDiagnostics(const std::string& uri) {
        std::lock_guard<std::mutex> lock(diagnosticsMutex_);
        auto it = diagnostics_.find(uri);
        if (it != diagnostics_.end()) {
            return it->second;
        }
        return {};
    }
    
    std::string formatDocument(const std::string& uri) {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            // Basic formatting - just return the original text for now
            return it->second;
        }
        return "";
    }
    
    std::string formatRange(const std::string& uri, const Range& range) {
        return formatDocument(uri); // Simplified implementation
    }
    
    std::string processMessage(const std::string& jsonMessage) {
        // Parse and handle LSP JSON-RPC messages
        llvm::Expected<llvm::json::Value> parsed = llvm::json::parse(jsonMessage);
        if (!parsed) {
            return R"({"error":{"code":-32700,"message":"Parse error"}})";
        }
        
        llvm::json::Object response;
        response["jsonrpc"] = "2.0";
        
        auto* obj = parsed->getAsObject();
        if (!obj) {
            response["error"] = llvm::json::Object{{"code", -32600}, {"message", "Invalid Request"}};
        } else {
            auto method = obj->getString("method");
            auto id = obj->get("id");
            
            if (id) {
                response["id"] = *id;
            }
            
            if (method) {
                if (*method == "initialize") {
                    response["result"] = createInitializeResult();
                } else if (*method == "shutdown") {
                    response["result"] = nullptr;
                } else {
                    response["error"] = llvm::json::Object{{"code", -32601}, {"message", "Method not found"}};
                }
            }
        }
        
        std::string result;
        llvm::raw_string_ostream os(result);
        os << llvm::json::Value(std::move(response));
        return result;
    }
    
    void setMessageCallback(std::function<void(const std::string&)> callback) {
        messageCallback_ = callback;
    }

private:
    void runDiagnostics(const std::string& uri) {
        std::vector<Diagnostic> diags;
        
        // Basic syntax checking would go here
        // For now, just clear any existing diagnostics
        
        std::lock_guard<std::mutex> lock(diagnosticsMutex_);
        diagnostics_[uri] = diags;
        
        // Notify callback if set
        if (messageCallback_) {
            std::string notification = createDiagnosticsNotification(uri, diags);
            messageCallback_(notification);
        }
    }
    
    llvm::json::Object createInitializeResult() {
        llvm::json::Object capabilities;
        capabilities["textDocumentSync"] = 1; // Full sync
        capabilities["completionProvider"] = llvm::json::Object{{"triggerCharacters", llvm::json::Array{".", "->", "::"}}};
        capabilities["hoverProvider"] = true;
        capabilities["definitionProvider"] = true;
        capabilities["referencesProvider"] = true;
        capabilities["documentFormattingProvider"] = true;
        capabilities["documentRangeFormattingProvider"] = true;
        
        llvm::json::Object serverInfo;
        serverInfo["name"] = "Miko C/C++ LSP";
        serverInfo["version"] = "1.0.0";
        
        llvm::json::Object result;
        result["capabilities"] = std::move(capabilities);
        result["serverInfo"] = std::move(serverInfo);
        
        return result;
    }
    
    std::string createDiagnosticsNotification(const std::string& uri, const std::vector<Diagnostic>& diagnostics) {
        llvm::json::Object notification;
        notification["jsonrpc"] = "2.0";
        notification["method"] = "textDocument/publishDiagnostics";
        
        llvm::json::Array diagArray;
        for (const auto& diag : diagnostics) {
            llvm::json::Object diagObj;
            diagObj["range"] = llvm::json::Object{
                {"start", llvm::json::Object{{"line", diag.range.start.line}, {"character", diag.range.start.character}}},
                {"end", llvm::json::Object{{"line", diag.range.end.line}, {"character", diag.range.end.character}}}
            };
            diagObj["severity"] = diag.severity;
            diagObj["message"] = diag.message;
            diagObj["source"] = diag.source;
            diagArray.push_back(std::move(diagObj));
        }
        
        llvm::json::Object params;
        params["uri"] = uri;
        params["diagnostics"] = std::move(diagArray);
        notification["params"] = std::move(params);
        
        std::string result;
        llvm::raw_string_ostream os(result);
        os << llvm::json::Value(std::move(notification));
        return result;
    }
    
    bool initialized_;
    bool shutdown_;
    std::string rootPath_;
    LSPCapabilities capabilities_;
    
    std::unique_ptr<clang::tooling::CompilationDatabase> compilationDatabase_;
    
    std::map<std::string, std::string> documents_;
    std::mutex documentsMutex_;
    
    std::map<std::string, std::vector<Diagnostic>> diagnostics_;
    std::mutex diagnosticsMutex_;
    
    std::function<void(const std::string&)> messageCallback_;
};

// LSPServer implementation
LSPServer::LSPServer() : pImpl(std::make_unique<Impl>()) {}

LSPServer::~LSPServer() = default;

bool LSPServer::initialize(const std::string& rootPath, const LSPCapabilities& capabilities) {
    return pImpl->initialize(rootPath, capabilities);
}

void LSPServer::shutdown() {
    pImpl->shutdown();
}

bool LSPServer::didOpen(const std::string& uri, const std::string& languageId, const std::string& text) {
    return pImpl->didOpen(uri, languageId, text);
}

bool LSPServer::didChange(const std::string& uri, const std::string& text) {
    return pImpl->didChange(uri, text);
}

bool LSPServer::didSave(const std::string& uri) {
    return pImpl->didSave(uri);
}

bool LSPServer::didClose(const std::string& uri) {
    return pImpl->didClose(uri);
}

std::vector<CompletionItem> LSPServer::completion(const TextDocumentPositionParams& params) {
    return pImpl->completion(params);
}

HoverResult LSPServer::hover(const TextDocumentPositionParams& params) {
    return pImpl->hover(params);
}

std::vector<Range> LSPServer::definition(const TextDocumentPositionParams& params) {
    return pImpl->definition(params);
}

std::vector<Range> LSPServer::references(const TextDocumentPositionParams& params) {
    return pImpl->references(params);
}

std::vector<Diagnostic> LSPServer::diagnostics(const std::string& uri) {
    return pImpl->getDiagnostics(uri);
}

std::string LSPServer::formatDocument(const std::string& uri) {
    return pImpl->formatDocument(uri);
}

std::string LSPServer::formatRange(const std::string& uri, const Range& range) {
    return pImpl->formatRange(uri, range);
}

std::string LSPServer::processMessage(const std::string& jsonMessage) {
    return pImpl->processMessage(jsonMessage);
}

void LSPServer::setMessageCallback(std::function<void(const std::string&)> callback) {
    pImpl->setMessageCallback(callback);
}

// C API implementation
extern "C" {
    LSPServer* lsp_create_server() {
        return new LSPServer();
    }
    
    void lsp_destroy_server(LSPServer* server) {
        delete server;
    }
    
    bool lsp_initialize(LSPServer* server, const char* rootPath) {
        if (!server || !rootPath) return false;
        LSPCapabilities caps; // Use default capabilities
        return server->initialize(std::string(rootPath), caps);
    }
    
    void lsp_shutdown(LSPServer* server) {
        if (server) server->shutdown();
    }
    
    bool lsp_did_open(LSPServer* server, const char* uri, const char* languageId, const char* text) {
        if (!server || !uri || !languageId || !text) return false;
        return server->didOpen(std::string(uri), std::string(languageId), std::string(text));
    }
    
    bool lsp_did_change(LSPServer* server, const char* uri, const char* text) {
        if (!server || !uri || !text) return false;
        return server->didChange(std::string(uri), std::string(text));
    }
    
    bool lsp_did_save(LSPServer* server, const char* uri) {
        if (!server || !uri) return false;
        return server->didSave(std::string(uri));
    }
    
    bool lsp_did_close(LSPServer* server, const char* uri) {
        if (!server || !uri) return false;
        return server->didClose(std::string(uri));
    }
    
    char* lsp_completion(LSPServer* server, const char* uri, int line, int character) {
        if (!server || !uri) return nullptr;
        
        TextDocumentPositionParams params;
        params.textDocument.uri = std::string(uri);
        params.position.line = line;
        params.position.character = character;
        
        auto items = server->completion(params);
        
        // Convert to JSON string
        llvm::json::Array jsonArray;
        for (const auto& item : items) {
            llvm::json::Object obj;
            obj["label"] = item.label;
            obj["detail"] = item.detail;
            obj["documentation"] = item.documentation;
            obj["insertText"] = item.insertText;
            obj["kind"] = item.kind;
            jsonArray.push_back(std::move(obj));
        }
        
        std::string result;
        llvm::raw_string_ostream os(result);
        os << llvm::json::Value(std::move(jsonArray));
        
        char* cstr = new char[result.length() + 1];
        std::strcpy(cstr, result.c_str());
        return cstr;
    }
    
    char* lsp_hover(LSPServer* server, const char* uri, int line, int character) {
        if (!server || !uri) return nullptr;
        
        TextDocumentPositionParams params;
        params.textDocument.uri = std::string(uri);
        params.position.line = line;
        params.position.character = character;
        
        auto hover = server->hover(params);
        
        llvm::json::Object obj;
        obj["contents"] = hover.contents;
        obj["range"] = llvm::json::Object{
            {"start", llvm::json::Object{{"line", hover.range.start.line}, {"character", hover.range.start.character}}},
            {"end", llvm::json::Object{{"line", hover.range.end.line}, {"character", hover.range.end.character}}}
        };
        
        std::string result;
        llvm::raw_string_ostream os(result);
        os << llvm::json::Value(std::move(obj));
        
        char* cstr = new char[result.length() + 1];
        std::strcpy(cstr, result.c_str());
        return cstr;
    }
    
    char* lsp_definition(LSPServer* server, const char* uri, int line, int character) {
        if (!server || !uri) return nullptr;
        
        TextDocumentPositionParams params;
        params.textDocument.uri = std::string(uri);
        params.position.line = line;
        params.position.character = character;
        
        auto ranges = server->definition(params);
        
        llvm::json::Array jsonArray;
        for (const auto& range : ranges) {
            llvm::json::Object obj;
            obj["start"] = llvm::json::Object{{"line", range.start.line}, {"character", range.start.character}};
            obj["end"] = llvm::json::Object{{"line", range.end.line}, {"character", range.end.character}};
            jsonArray.push_back(std::move(obj));
        }
        
        std::string result;
        llvm::raw_string_ostream os(result);
        os << llvm::json::Value(std::move(jsonArray));
        
        char* cstr = new char[result.length() + 1];
        std::strcpy(cstr, result.c_str());
        return cstr;
    }
    
    char* lsp_references(LSPServer* server, const char* uri, int line, int character) {
        return lsp_definition(server, uri, line, character); // Same implementation for now
    }
    
    char* lsp_diagnostics(LSPServer* server, const char* uri) {
        if (!server || !uri) return nullptr;
        
        auto diagnostics = server->diagnostics(std::string(uri));
        
        llvm::json::Array jsonArray;
        for (const auto& diag : diagnostics) {
            llvm::json::Object obj;
            obj["range"] = llvm::json::Object{
                {"start", llvm::json::Object{{"line", diag.range.start.line}, {"character", diag.range.start.character}}},
                {"end", llvm::json::Object{{"line", diag.range.end.line}, {"character", diag.range.end.character}}}
            };
            obj["severity"] = diag.severity;
            obj["message"] = diag.message;
            obj["source"] = diag.source;
            jsonArray.push_back(std::move(obj));
        }
        
        std::string result;
        llvm::raw_string_ostream os(result);
        os << llvm::json::Value(std::move(jsonArray));
        
        char* cstr = new char[result.length() + 1];
        std::strcpy(cstr, result.c_str());
        return cstr;
    }
    
    char* lsp_format_document(LSPServer* server, const char* uri) {
        if (!server || !uri) return nullptr;
        
        auto formatted = server->formatDocument(std::string(uri));
        
        char* cstr = new char[formatted.length() + 1];
        std::strcpy(cstr, formatted.c_str());
        return cstr;
    }
    
    char* lsp_format_range(LSPServer* server, const char* uri, int startLine, int startChar, int endLine, int endChar) {
        if (!server || !uri) return nullptr;
        
        Range range;
        range.start.line = startLine;
        range.start.character = startChar;
        range.end.line = endLine;
        range.end.character = endChar;
        
        auto formatted = server->formatRange(std::string(uri), range);
        
        char* cstr = new char[formatted.length() + 1];
        std::strcpy(cstr, formatted.c_str());
        return cstr;
    }
    
    char* lsp_process_message(LSPServer* server, const char* jsonMessage) {
        if (!server || !jsonMessage) return nullptr;
        
        auto response = server->processMessage(std::string(jsonMessage));
        
        char* cstr = new char[response.length() + 1];
        std::strcpy(cstr, response.c_str());
        return cstr;
    }
    
    void lsp_free_string(char* str) {
        delete[] str;
    }
}

} // namespace lsp
} // namespace miko