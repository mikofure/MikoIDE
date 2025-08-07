import { createSignal, For, onMount, createEffect, Show } from "solid-js";
import { Download, Star, Settings, Search, Image as ImageIcon } from "lucide-solid";

interface Extension {
    extensionId: string;
    extensionName: string;
    displayName: string;
    shortDescription: string;
    publisher: {
        publisherName: string;
        displayName: string;
    };
    versions: Array<{
        version: string;
        assetUri: string;
        files: Array<{
            assetType: string;
            source: string;
        }>;
    }>;
    statistics: Array<{
        statisticName: string;
        value: number;
    }>;
    installed?: boolean;
    enabled?: boolean;
}

interface MarketplaceResponse {
    results: Array<{
        extensions: Extension[];
        resultMetadata: Array<{
            metadataType: string;
            metadataItems: Array<{
                name: string;
                count: number;
            }>;
        }>;
    }>;
}

function ExtensionsPage() {
    const [searchTerm, setSearchTerm] = createSignal("");
    const [activeTab, setActiveTab] = createSignal<'installed' | 'marketplace'>('marketplace');
    const [extensions, setExtensions] = createSignal<Extension[]>([]);
    const [loading, setLoading] = createSignal(false);
    const [error, setError] = createSignal<string | null>(null);
    const [page, setPage] = createSignal(1);
    const [hasMore, setHasMore] = createSignal(true);
    const [sortBy, setSortBy] = createSignal(0);
    const [selectedCategory, setSelectedCategory] = createSignal<string>("");
    const [installedExtensions, setInstalledExtensions] = createSignal<Extension[]>([
        {
            extensionId: "ms-python.python",
            extensionName: "python",
            displayName: "Python",
            shortDescription: "IntelliSense (Pylance), Linting, Debugging (multi-threaded, remote), Jupyter Notebooks, code formatting, refactoring, unit tests, and more.",
            publisher: {
                publisherName: "ms-python",
                displayName: "Microsoft"
            },
            versions: [{
                version: "2023.20.0",
                assetUri: "",
                files: []
            }],
            statistics: [
                { statisticName: "install", value: 75000000 },
                { statisticName: "averagerating", value: 4.5 }
            ],
            installed: true,
            enabled: true
        }
    ]);

    const API_URL = import.meta.env.VITE_VSMKT_API;

    const fetchExtensions = async (
        searchQuery: string = "", 
        pageNum: number = 1, 
        append: boolean = false,
        category?: string,
        sortByOption: number = 0
    ) => {
        setLoading(true);
        setError(null);

        try {
            const criteria = [
                { filterType: 8, value: "Microsoft.VisualStudio.Code" },
                { filterType: 12, value: "37888" }
            ];

            if (searchQuery) {
                criteria.push({ filterType: 10, value: searchQuery });
            }

            if (category) {
                criteria.push({ filterType: 5, value: category });
            }

            const requestBody = {
                filters: [{
                    criteria,
                    pageNumber: pageNum,
                    pageSize: 20,
                    sortBy: sortByOption,
                    sortOrder: 0
                }],
                assetTypes: [
                    "Microsoft.VisualStudio.Services.Icons.Default",
                    "Microsoft.VisualStudio.Services.Icons.Small",
                    "Microsoft.VisualStudio.Services.Icons.Large"
                ],
                flags: 914
            };

            const response = await fetch(API_URL, {
                method: 'POST',
                headers: {
                    'Accept': 'application/json;api-version=3.0-preview.1',
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(requestBody)
            });

            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }

            const data: MarketplaceResponse = await response.json();
            const newExtensions = data.results[0]?.extensions || [];

            if (append) {
                setExtensions(prev => [...prev, ...newExtensions]);
            } else {
                setExtensions(newExtensions);
            }

            setHasMore(newExtensions.length === 20);
        } catch (err) {
            setError(err instanceof Error ? err.message : 'Failed to fetch extensions');
            console.error('Error fetching extensions:', err);
        } finally {
            setLoading(false);
        }
    };
    //@ts-expect-error
    const fetchExtensionById = async (extensionId: string) => {
        setLoading(true);
        setError(null);

        try {
            const requestBody = {
                filters: [{
                    criteria: [
                        { filterType: 7, value: extensionId }
                    ]
                }],
                assetTypes: [
                    "Microsoft.VisualStudio.Services.Icons.Default",
                    "Microsoft.VisualStudio.Services.Icons.Small",
                    "Microsoft.VisualStudio.Services.Icons.Large"
                ],
                flags: 914
            };

            const response = await fetch(API_URL, {
                method: 'POST',
                headers: {
                    'Accept': 'application/json;api-version=3.0-preview.1',
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(requestBody)
            });

            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }

            const data: MarketplaceResponse = await response.json();
            return data.results[0]?.extensions[0] || null;
        } catch (err) {
            console.error('Error fetching extension by ID:', err);
            return null;
        } finally {
            setLoading(false);
        }
    };

    const getExtensionIcon = (extension: Extension): string => {
        if (!extension?.versions?.[0]?.files) return '/default-extension-icon.png';
        
        const assetTypes = ['Default', 'Small', 'Large'];
        
        for (const assetType of assetTypes) {
            const iconFile = extension.versions[0].files.find(
              file => file.assetType === `Microsoft.VisualStudio.Services.Icons.${assetType}`
            );
            if (iconFile?.source) {
              return iconFile.source;
            }
        }
        
        return '/default-extension-icon.png';
    };

    const getDownloadCount = (extension: Extension): number => {
        return extension.statistics?.find(s => s.statisticName === "install")?.value || 0;
    };

    const getRating = (extension: Extension): number => {
        return extension.statistics?.find(s => s.statisticName === "averagerating")?.value || 0;
    };

    const isInstalled = (extensionId: string): boolean => {
        return installedExtensions().some(ext => ext.extensionId === extensionId);
    };

    const isEnabled = (extensionId: string): boolean => {
        const ext = installedExtensions().find(ext => ext.extensionId === extensionId);
        return ext?.enabled || false;
    };

    const toggleExtension = (extensionId: string) => {
        setInstalledExtensions(prev => prev.map(ext =>
            ext.extensionId === extensionId ? { ...ext, enabled: !ext.enabled } : ext
        ));
    };

    const installExtension = (extension: Extension) => {
        const newExt = { ...extension, installed: true, enabled: true };
        setInstalledExtensions(prev => [...prev, newExt]);
    };

    const uninstallExtension = (extensionId: string) => {
        setInstalledExtensions(prev => prev.filter(ext => ext.extensionId !== extensionId));
    };

    const loadMore = () => {
        if (!loading() && hasMore()) {
            const nextPage = page() + 1;
            setPage(nextPage);
            fetchExtensions(searchTerm(), nextPage, true, selectedCategory(), sortBy());
        }
    };

    const handleScroll = (e: Event) => {
        const target = e.target as HTMLElement;
        const { scrollTop, scrollHeight, clientHeight } = target;

        if (scrollHeight - scrollTop <= clientHeight + 100) {
            loadMore();
        }
    };

    const formatNumber = (num: number): string => {
        if (num >= 1_000_000_000) return (num / 1_000_000_000).toFixed(1).replace(/\.0$/, '') + 'B';
        if (num >= 1_000_000) return (num / 1_000_000).toFixed(1).replace(/\.0$/, '') + 'M';
        if (num >= 1_000) return (num / 1_000).toFixed(1).replace(/\.0$/, '') + 'K';
        return num.toString();
    };

    // Search with debounce
    createEffect(() => {
        const term = searchTerm();
        const timeoutId = setTimeout(() => {
            if (activeTab() === 'marketplace') {
                setPage(1);
                fetchExtensions(term, 1, false, selectedCategory(), sortBy());
            }
        }, 300);

        return () => clearTimeout(timeoutId);
    });

    onMount(() => {
        if (activeTab() === 'marketplace') {
            fetchExtensions();
        }
    });

    const filteredExtensions = () => {
        if (activeTab() === 'installed') {
            const term = searchTerm().toLowerCase();
            return installedExtensions().filter(ext =>
                ext.displayName.toLowerCase().includes(term) ||
                ext.shortDescription.toLowerCase().includes(term)
            );
        }
        return extensions();
    };

    return (
        <div class="h-full flex flex-col">
            <div class="p-2 border-b border-neutral-800">
                <h3 class="text-xs font-medium text-gray-300 uppercase tracking-wide mb-3">Extensions</h3>

                {/* Search */}
                <div class="relative mb-3">
                    <Search size={10} class="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400" />
                    <input
                        type="text"
                        placeholder="Search extensions"
                        value={searchTerm()}
                        onInput={(e) => setSearchTerm(e.currentTarget.value)}
                        class="w-full pl-7 pr-2 py-1 placeholder:text-xs bg-neutral-800 border border-neutral-700 rounded text-sm text-gray-300 placeholder-gray-500 focus:outline-none focus:border-blue-500"
                    />
                </div>

                {/* Sort and Filter Controls */}
                <Show when={activeTab() === 'marketplace'}>
                    <div class="flex gap-2 mb-3">
                        {/* Sort Dropdown */}
                        <select
                            value={sortBy()}
                            onChange={(e) => {
                                const newSortBy = parseInt(e.currentTarget.value);
                                setSortBy(newSortBy);
                                setPage(1);
                                fetchExtensions(searchTerm(), 1, false, selectedCategory(), newSortBy);
                            }}
                            class="flex-1 px-2 py-1 text-xs bg-neutral-800 border border-neutral-700 rounded text-gray-300 focus:outline-none focus:border-blue-500"
                        >
                            <option value={0}>Relevance</option>
                            <option value={4}>Rating</option>
                            <option value={5}>Downloads</option>
                            <option value={6}>Name</option>
                        </select>

                        {/* Category Dropdown */}
                        <select
                            value={selectedCategory()}
                            onChange={(e) => {
                                const newCategory = e.currentTarget.value;
                                setSelectedCategory(newCategory);
                                setPage(1);
                                fetchExtensions(searchTerm(), 1, false, newCategory, sortBy());
                            }}
                            class="flex-1 px-2 py-1 text-xs bg-neutral-800 border border-neutral-700 rounded text-gray-300 focus:outline-none focus:border-blue-500"
                        >
                            <option value="">All Categories</option>
                            <option value="Programming Languages">Languages</option>
                            <option value="Debuggers">Debuggers</option>
                            <option value="Formatters">Formatters</option>
                            <option value="Themes">Themes</option>
                            <option value="Snippets">Snippets</option>
                            <option value="Linters">Linters</option>
                        </select>
                    </div>
                </Show>

                {/* Tabs */}
                <div class="flex bg-neutral-800 rounded p-1">
                    <button
                        onClick={() => {
                            setActiveTab('installed');
                            setExtensions([]);
                            setPage(1);
                        }}
                        class={`flex-1 px-3 py-1 text-xs rounded transition-colors ${
                            activeTab() === 'installed'
                                ? 'bg-blue-600 text-white'
                                : 'text-gray-400 hover:text-gray-300'
                        }`}
                    >
                        Installed
                    </button>
                    <button
                        onClick={() => {
                            setActiveTab('marketplace');
                            setPage(1);
                            if (extensions().length === 0) {
                                fetchExtensions(searchTerm(), 1, false, selectedCategory(), sortBy());
                            }
                        }}
                        class={`flex-1 px-3 py-1 text-xs rounded transition-colors ${
                            activeTab() === 'marketplace'
                                ? 'bg-blue-600 text-white'
                                : 'text-gray-400 hover:text-gray-300'
                        }`}
                    >
                        Marketplace
                    </button>
                </div>
            </div>

            <div class="flex-1 overflow-y-auto" onScroll={handleScroll}>
                <Show when={error()}>
                    <div class="p-3 text-red-400 text-sm bg-red-900/20 border-b border-red-800">
                        Error: {error()}
                    </div>
                </Show>

                <For each={filteredExtensions()}>
                    {(extension) => {
                        const iconUrl = getExtensionIcon(extension);
                        const downloadCount = getDownloadCount(extension);
                        const rating = getRating(extension);
                        const installed = isInstalled(extension.extensionId);
                        const enabled = isEnabled(extension.extensionId);

                        return (
                            <div class="p-3 border-b border-neutral-800/50 hover:bg-neutral-800/30">
                                <div class="flex items-start gap-3 mb-2">
                                    {/* Extension Icon */}
                                    <div class="w-10 h-10 flex-shrink-0 rounded overflow-hidden">
                                        <Show
                                            when={iconUrl}
                                            fallback={
                                                <div class="w-full h-full flex items-center justify-center">
                                                    <ImageIcon size={16} class="text-gray-500" />
                                                </div>
                                            }
                                        >
                                            <img
                                                src={iconUrl}
                                                alt={extension.displayName}
                                                class="w-full h-full object-cover"
                                                loading="lazy"
                                                onError={(e) => {
                                                    const target = e.target as HTMLImageElement;
                                                    target.style.display = 'none';
                                                }}
                                            />
                                        </Show>
                                    </div>

                                    {/* Extension Info */}
                                    <div class="flex-1 min-w-0">
                                        <div class="flex items-start justify-between mb-1">
                                            <div class="flex-1 min-w-0">
                                                <h4 class="text-sm font-medium text-gray-300 truncate">
                                                    {extension.displayName}
                                                </h4>
                                                <p class="text-xs text-gray-500">
                                                    by {extension.publisher.displayName}
                                                </p>
                                            </div>

                                            {/* Action Buttons */}
                                            <div class="flex items-center gap-1 ml-2">
                                                <Show when={installed}>
                                                    <button
                                                        onClick={() => toggleExtension(extension.extensionId)}
                                                        class={`px-2 py-1 text-xs rounded ${
                                                            enabled
                                                                ? 'bg-green-600 text-white'
                                                                : 'bg-gray-600 text-gray-300'
                                                        }`}
                                                    >
                                                        {enabled ? 'Enabled' : 'Disabled'}
                                                    </button>
                                                    <button
                                                        onClick={() => uninstallExtension(extension.extensionId)}
                                                        class="p-1 text-gray-400 hover:text-red-400"
                                                        title="Uninstall"
                                                    >
                                                        <Settings size={12} />
                                                    </button>
                                                </Show>
                                                <Show when={!installed}>
                                                    <button
                                                        onClick={() => installExtension(extension)}
                                                        class="px-2 py-1 text-xs bg-blue-600 hover:bg-blue-700 text-white rounded transition-colors"
                                                    >
                                                        <Download size={12} class="inline mr-1" />
                                                        Install
                                                    </button>
                                                </Show>
                                            </div>
                                        </div>

                                        {/* Description */}
                                        <p class="text-xs text-gray-400 mb-2 line-clamp-2">
                                            {extension.shortDescription}
                                        </p>

                                        {/* Stats */}
                                        <div class="flex justify-between text-xs text-gray-500">
                                            <span class="text-gray-400 truncate">v{extension.versions[0]?.version}</span>

                                            <div class="flex items-center gap-4">
                                                <Show when={rating > 0}>
                                                    <div class="flex items-center gap-1">
                                                        <Star size={10} class="text-yellow-400" />
                                                        <span>{rating.toFixed(1)}</span>
                                                    </div>
                                                </Show>
                                                <Show when={downloadCount > 0}>
                                                    <div class="flex items-center gap-1">
                                                        <Download size={10} class="text-blue-400" />
                                                        <span>{formatNumber(downloadCount)}</span>
                                                    </div>
                                                </Show>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        );
                    }}
                </For>

                <Show when={loading()}>
                    <div class="p-4 text-center text-gray-400 text-sm">
                        Loading extensions...
                    </div>
                </Show>

                <Show when={!loading() && !hasMore() && activeTab() === 'marketplace' && extensions().length > 0}>
                    <div class="p-4 text-center text-gray-500 text-sm">
                        No more extensions to load
                    </div>
                </Show>

                <Show when={!loading() && filteredExtensions().length === 0}>
                    <div class="p-4 text-center text-gray-500 text-sm">
                        {activeTab() === 'installed' ? 'No installed extensions found' : 'No extensions found'}
                    </div>
                </Show>
            </div>
        </div>
    );
}

export default ExtensionsPage;
