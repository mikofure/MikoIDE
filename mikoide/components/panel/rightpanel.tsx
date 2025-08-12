import { createSignal, onCleanup, For, Show } from "solid-js";
import { Send, Lightbulb, RotateCcw } from "lucide-solid";

interface RightPanelProps {
    width: number;
    onResize: (newWidth: number) => void;
}

interface ChatMessage {
    id: string;
    type: 'user' | 'assistant';
    content: string;
    timestamp: Date;
    isTyping?: boolean;
}

function RightPanel(props: RightPanelProps) {
    let resizer: HTMLDivElement | undefined;
    let isResizing = false;
    let chatContainer: HTMLDivElement | undefined;
    let messageInput: HTMLTextAreaElement | undefined;

    const [messages, setMessages] = createSignal<ChatMessage[]>([
        {
            id: '1',
            type: 'assistant',
            content: 'Hello! I\'m Miko Codehelper AI, your intelligent coding assistant. I can help you with:\n\n• Code explanations and debugging\n• Best practices and optimization\n• Documentation and comments\n• Refactoring suggestions\n• Language-specific questions\n\nHow can I assist you today?',
            timestamp: new Date()
        }
    ]);

    const [inputValue, setInputValue] = createSignal('');
    const [isLoading, setIsLoading] = createSignal(false);
    const [suggestions] = createSignal([
        'Explain this code',
        'Find bugs in my code',
        'Optimize performance',
        'Add documentation',
        'Refactor this function',
        'Best practices'
    ]);

    const onMouseDown = (e: MouseEvent) => {
        isResizing = true;
        e.preventDefault();
    };

    const onMouseMove = (e: MouseEvent) => {
        if (!isResizing) return;
        const newWidth = window.innerWidth - e.clientX;
        if (newWidth > 250 && newWidth < 600) {
            props.onResize(newWidth);
        }
    };

    const onMouseUp = () => {
        isResizing = false;
    };

    // Global events
    window.addEventListener("mousemove", onMouseMove);
    window.addEventListener("mouseup", onMouseUp);

    onCleanup(() => {
        window.removeEventListener("mousemove", onMouseMove);
        window.removeEventListener("mouseup", onMouseUp);
    });

    const scrollToBottom = () => {
        if (chatContainer) {
            chatContainer.scrollTop = chatContainer.scrollHeight;
        }
    };

    const sendMessage = async () => {
        const message = inputValue().trim();
        if (!message || isLoading()) return;

        const userMessage: ChatMessage = {
            id: Date.now().toString(),
            type: 'user',
            content: message,
            timestamp: new Date()
        };

        setMessages(prev => [...prev, userMessage]);
        setInputValue('');
        setIsLoading(true);

        setTimeout(scrollToBottom, 10);

        // Simulate AI response
        setTimeout(() => {
            const responses = [
                'I\'d be happy to help you with that! Could you share the specific code you\'re working with?',
                'That\'s a great question! Here are some suggestions based on best practices...',
                'I can help you optimize that. Let me analyze the code structure and suggest improvements.',
                'For debugging, I recommend checking these common issues first...',
                'Here\'s how you can refactor this code to make it more maintainable...',
                'Let me explain this concept step by step to make it clearer...'
            ];

            const aiMessage: ChatMessage = {
                id: (Date.now() + 1).toString(),
                type: 'assistant',
                content: responses[Math.floor(Math.random() * responses.length)],
                timestamp: new Date()
            };

            setMessages(prev => [...prev, aiMessage]);
            setIsLoading(false);
            setTimeout(scrollToBottom, 10);
        }, 1000 + Math.random() * 2000);
    };

    const handleKeyDown = (e: KeyboardEvent) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    };

    const clearChat = () => {
        setMessages([
            {
                id: '1',
                type: 'assistant',
                content: 'Chat cleared! How can I help you with your code today?',
                timestamp: new Date()
            }
        ]);
    };

    const useSuggestion = (suggestion: string) => {
        setInputValue(suggestion);
        messageInput?.focus();
    };

    return (
        <div
            class="flex h-full bg-neutral-900 rounded-[8px] relative border border-neutral-800 select-none flex-col"
            style={{ width: `${props.width}px` }}
        >
            {/* Resize handle */}
            <div
                ref={resizer}
                class="absolute left-0 top-0 w-1 h-full cursor-col-resize hover:bg-blue-500/50 transition-colors"
                onMouseDown={onMouseDown}
            />

            {/* Header */}
            <div class="flex items-center justify-between p-3 border-b border-neutral-800">
                <div class="flex items-center gap-2">
                    <div>
                        <h3 class="text-sm font-medium text-white">Miko Codehelper AI</h3>
                        <p class="text-xs text-neutral-400">Intelligent Coding Assistant</p>
                    </div>
                </div>
                <button
                    onClick={clearChat}
                    class="p-1.5 hover:bg-neutral-800 rounded-md transition-colors"
                    title="Clear chat"
                >
                    <RotateCcw size={14} class="text-neutral-400" />
                </button>
            </div>

            {/* Chat messages */}
            <div
                ref={chatContainer}
                class="flex-1 overflow-y-auto p-3 space-y-3 scrollbar-thin scrollbar-thumb-neutral-700 scrollbar-track-transparent"
            >
                <For each={messages()}>
                    {(message) => (
                        <div class={`flex ${message.type === 'user' ? 'justify-end' : 'justify-start'}`}>
                            <div class={`max-w-[85%] rounded-lg p-3 ${message.type === 'user'
                                ? 'bg-blue-600 text-white'
                                : 'bg-neutral-800 text-neutral-100'
                                }`}>
                                <div class="text-xs whitespace-pre-wrap">{message.content}</div>
                                <div class={`text-xs mt-1 ${message.type === 'user' ? 'text-blue-200' : 'text-neutral-500'
                                    }`}>
                                    {message.timestamp.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
                                </div>
                            </div>
                        </div>
                    )}
                </For>

                <Show when={isLoading()}>
                    <div class="flex justify-start">
                        <div class="bg-neutral-800 rounded-lg p-3">
                            <div class="flex items-center gap-2">
                                <div class="flex space-x-1">
                                    <div class="w-2 h-2 bg-blue-500 rounded-full animate-bounce" style="animation-delay: 0ms" />
                                    <div class="w-2 h-2 bg-blue-500 rounded-full animate-bounce" style="animation-delay: 150ms" />
                                    <div class="w-2 h-2 bg-blue-500 rounded-full animate-bounce" style="animation-delay: 300ms" />
                                </div>
                                <span class="text-xs text-neutral-400">Miko is thinking...</span>
                            </div>
                        </div>
                    </div>
                </Show>
            </div>

            {/* Quick suggestions */}
            <div class="px-3 py-2 border-t border-neutral-800">
                <div class="text-xs text-neutral-400 mb-2 flex items-center gap-1">
                    <Lightbulb size={12} />
                    Quick suggestions:
                </div>
                <div class="flex flex-wrap gap-1">
                    <For each={suggestions().slice(0, 3)}>
                        {(suggestion) => (
                            <button
                                onClick={() => useSuggestion(suggestion)}
                                class="text-xs px-2 py-1 bg-neutral-800 hover:bg-neutral-700 rounded-md text-neutral-300 transition-colors"
                            >
                                {suggestion}
                            </button>
                        )}
                    </For>
                </div>
            </div>

            {/* Input area */}
            <div class="p-3 border-t border-neutral-800">
                <div class="flex gap-2">
                    <div class="flex-1 flex bg-neutral-800 border border-neutral-700 rounded-lg">
                        <textarea
                            ref={messageInput}
                            value={inputValue()}
                            onInput={(e) => setInputValue(e.currentTarget.value)}
                            onKeyDown={handleKeyDown}
                            placeholder="Ask Miko about your code..."
                            class="w-full  px-3 py-2 text-sm text-white placeholder-neutral-400 resize-none focus:outline-none focus:border-blue-500 transition-colors placeholder:text-xs"
                            rows={2}
                            disabled={isLoading()}
                        />
                        <button
                            onClick={sendMessage}
                            disabled={!inputValue().trim() || isLoading()}
                            class="px-3 py-2 disabled:cursor-not-allowed rounded-lg transition-colors flex items-center justify-center"
                        >
                            <Send size={16} class="text-white" />
                        </button>
                    </div>

                </div>

            </div>
        </div>
    );
}

export default RightPanel;