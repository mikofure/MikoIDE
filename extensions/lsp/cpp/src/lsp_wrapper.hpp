#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

#ifdef _WIN32
    #ifdef LSP_WRAPPER_EXPORTS
        #define LSP_API __declspec(dllexport)
    #else
        #define LSP_API __declspec(dllimport)
    #endif
#else
    #define LSP_API __attribute__((visibility("default")))
#endif

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
    #define WASM_EXPORT
#endif

namespace miko {
namespace lsp {

// LSP Message Types
enum class MessageType {
    Request,
    Response,
    Notification
};

// LSP Request/Response structure
struct LSPMessage {
    MessageType type;
    std::string method;
    std::string id;
    std::string params;
    std::string result;
    std::string error;
};

// LSP Capabilities
struct LSPCapabilities {
    bool textDocumentSync = true;
    bool completion = true;
    bool hover = true;
    bool signatureHelp = true;
    bool definition = true;
    bool references = true;
    bool documentHighlight = true;
    bool documentSymbol = true;
    bool workspaceSymbol = true;
    bool codeAction = true;
    bool documentFormatting = true;
    bool documentRangeFormatting = true;
    bool rename = true;
    bool foldingRange = true;
    bool semanticTokens = true;
};

// Document position
struct Position {
    int line;
    int character;
};

// Document range
struct Range {
    Position start;
    Position end;
};

// Text document identifier
struct TextDocumentIdentifier {
    std::string uri;
};

// Text document position params
struct TextDocumentPositionParams {
    TextDocumentIdentifier textDocument;
    Position position;
};

// Completion item
struct CompletionItem {
    std::string label;
    std::string detail;
    std::string documentation;
    std::string insertText;
    int kind;
};

// Hover result
struct HoverResult {
    std::string contents;
    Range range;
};

// Diagnostic
struct Diagnostic {
    Range range;
    std::string message;
    int severity;
    std::string source;
};

// LSP Server Interface
class LSP_API LSPServer {
public:
    LSPServer();
    virtual ~LSPServer();

    // Initialize the LSP server
    bool initialize(const std::string& rootPath, const LSPCapabilities& capabilities);
    
    // Shutdown the server
    void shutdown();
    
    // Document lifecycle
    bool didOpen(const std::string& uri, const std::string& languageId, const std::string& text);
    bool didChange(const std::string& uri, const std::string& text);
    bool didSave(const std::string& uri);
    bool didClose(const std::string& uri);
    
    // Language features
    std::vector<CompletionItem> completion(const TextDocumentPositionParams& params);
    HoverResult hover(const TextDocumentPositionParams& params);
    std::vector<Range> definition(const TextDocumentPositionParams& params);
    std::vector<Range> references(const TextDocumentPositionParams& params);
    std::vector<Diagnostic> diagnostics(const std::string& uri);
    
    // Formatting
    std::string formatDocument(const std::string& uri);
    std::string formatRange(const std::string& uri, const Range& range);
    
    // Process LSP message
    std::string processMessage(const std::string& jsonMessage);
    
    // Set message callback for async responses
    void setMessageCallback(std::function<void(const std::string&)> callback);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// C-style API for DLL/WASM exports
extern "C" {
    // Server lifecycle
    LSP_API WASM_EXPORT LSPServer* lsp_create_server();
    LSP_API WASM_EXPORT void lsp_destroy_server(LSPServer* server);
    LSP_API WASM_EXPORT bool lsp_initialize(LSPServer* server, const char* rootPath);
    LSP_API WASM_EXPORT void lsp_shutdown(LSPServer* server);
    
    // Document operations
    LSP_API WASM_EXPORT bool lsp_did_open(LSPServer* server, const char* uri, const char* languageId, const char* text);
    LSP_API WASM_EXPORT bool lsp_did_change(LSPServer* server, const char* uri, const char* text);
    LSP_API WASM_EXPORT bool lsp_did_save(LSPServer* server, const char* uri);
    LSP_API WASM_EXPORT bool lsp_did_close(LSPServer* server, const char* uri);
    
    // Language features (returns JSON strings)
    LSP_API WASM_EXPORT char* lsp_completion(LSPServer* server, const char* uri, int line, int character);
    LSP_API WASM_EXPORT char* lsp_hover(LSPServer* server, const char* uri, int line, int character);
    LSP_API WASM_EXPORT char* lsp_definition(LSPServer* server, const char* uri, int line, int character);
    LSP_API WASM_EXPORT char* lsp_references(LSPServer* server, const char* uri, int line, int character);
    LSP_API WASM_EXPORT char* lsp_diagnostics(LSPServer* server, const char* uri);
    
    // Formatting
    LSP_API WASM_EXPORT char* lsp_format_document(LSPServer* server, const char* uri);
    LSP_API WASM_EXPORT char* lsp_format_range(LSPServer* server, const char* uri, int startLine, int startChar, int endLine, int endChar);
    
    // Message processing
    LSP_API WASM_EXPORT char* lsp_process_message(LSPServer* server, const char* jsonMessage);
    
    // Memory management
    LSP_API WASM_EXPORT void lsp_free_string(char* str);
}

} // namespace lsp
} // namespace miko