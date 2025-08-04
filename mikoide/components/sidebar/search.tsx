import { createSignal, For } from "solid-js";
import {  X, FileText, Replace } from "lucide-solid";

interface SearchResult {
    file: string;
    line: number;
    content: string;
    match: string;
}

function SearchPage() {
    const [searchTerm, setSearchTerm] = createSignal("");
    const [replaceTerm, setReplaceTerm] = createSignal("");
    const [showReplace, setShowReplace] = createSignal(false);
    const [caseSensitive, setCaseSensitive] = createSignal(false);
    const [wholeWord, setWholeWord] = createSignal(false);
    const [useRegex, setUseRegex] = createSignal(false);
    const [results, setResults] = createSignal<SearchResult[]>([
        {
            file: "src/main.tsx",
            line: 15,
            content: "import { render } from 'solid-js/web'",
            match: "render"
        },
        {
            file: "src/App.tsx",
            line: 8,
            content: "function App() {",
            match: "function"
        }
    ]);

    const handleSearch = () => {
        if (!searchTerm().trim()) {
            setResults([]);
            return;
        }
        // Mock search results - in real implementation, this would search through files
        console.log('Searching for:', searchTerm());
    };

    const clearSearch = () => {
        setSearchTerm("");
        setResults([]);
    };

    return (
        <div class="h-full flex flex-col">
            <div class="p-3 border-b border-neutral-800">
                <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide mb-3">Search</h3>
                
                {/* Search Input */}
                <div class="relative mb-2">
                    <input
                        type="text"
                        placeholder="Search"
                        value={searchTerm()}
                        onInput={(e) => setSearchTerm(e.currentTarget.value)}
                        onKeyPress={(e) => e.key === 'Enter' && handleSearch()}
                        class="w-full px-3 py-1 pr-8 placeholder:text-xs bg-neutral-800 border border-neutral-700 rounded text-sm text-gray-300 placeholder-gray-500 focus:outline-none focus:border-blue-500"
                    />
                    {searchTerm() && (
                        <button
                            onClick={clearSearch}
                            class="absolute right-2 top-1/2 transform -translate-y-1/2 text-gray-400 hover:text-gray-300"
                        >
                            <X size={14} />
                        </button>
                    )}
                </div>

                {/* Replace Input */}
                {showReplace() && (
                    <div class="relative mb-2">
                        <input
                            type="text"
                            placeholder="Replace"
                            value={replaceTerm()}
                            onInput={(e) => setReplaceTerm(e.currentTarget.value)}
                            class="w-full px-3 py-1 bg-neutral-800 border border-neutral-700 rounded text-sm text-gray-300 placeholder-gray-500 focus:outline-none focus:border-blue-500"
                        />
                    </div>
                )}

                {/* Search Options */}
                <div class="flex items-center gap-2 mb-2">
                    <button
                        onClick={() => setShowReplace(!showReplace())}
                        class={`p-1 rounded ${showReplace() ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-gray-300'}`}
                        title="Toggle Replace"
                    >
                        <Replace size={14} />
                    </button>
                    <button
                        onClick={() => setCaseSensitive(!caseSensitive())}
                        class={`px-2 py-1 text-xs rounded ${caseSensitive() ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-gray-300'}`}
                        title="Match Case"
                    >
                        Aa
                    </button>
                    <button
                        onClick={() => setWholeWord(!wholeWord())}
                        class={`px-2 py-1 text-xs rounded ${wholeWord() ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-gray-300'}`}
                        title="Match Whole Word"
                    >
                        Ab
                    </button>
                    <button
                        onClick={() => setUseRegex(!useRegex())}
                        class={`px-2 py-1 text-xs rounded ${useRegex() ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-gray-300'}`}
                        title="Use Regular Expression"
                    >
                        .*
                    </button>
                </div>
            </div>

            {/* Results */}
            <div class="flex-1 overflow-y-auto">
                {results().length > 0 ? (
                    <>
                        <div class="p-2 text-xs text-gray-400 border-b border-neutral-800">
                            {results().length} results in {new Set(results().map(r => r.file)).size} files
                        </div>
                        <For each={results()}>
                            {(result) => (
                                <div class="p-2 hover:bg-neutral-800 cursor-pointer border-b border-neutral-800/50">
                                    <div class="flex items-center gap-2 mb-1">
                                        <FileText size={12} class="text-blue-400" />
                                        <span class="text-xs text-gray-400">{result.file}</span>
                                        <span class="text-xs text-gray-500">:{result.line}</span>
                                    </div>
                                    <div class="text-xs text-gray-300 ml-5">
                                        {result.content}
                                    </div>
                                </div>
                            )}
                        </For>
                    </>
                ) : searchTerm() ? (
                    <div class="p-4 text-center text-gray-500 text-sm">
                        No results found
                    </div>
                ) : (
                    <div class="p-4 text-center text-gray-500 text-sm">
                        Search across files
                    </div>
                )}
            </div>
        </div>
    );
}

export default SearchPage;