import { ImageDown } from "lucide-solid";
import * as htmlToImage from "html-to-image";
import { saveAs } from "file-saver";
import { createSignal, onMount } from "solid-js";
//@ts-expect-error
import { Motion } from "@motionone/solid";

interface CaptureProps {
    onClose: () => void;
    targetElementId: string;
}

function Capture(props: CaptureProps) {
    const [previewUrl, setPreviewUrl] = createSignal<string>("");
    const [scale, setScale] = createSignal(2);
    const [isCapturing, setIsCapturing] = createSignal(false);

    // Generate preview when component mounts
    onMount(async () => {
        await generatePreview();
    });

    const generatePreview = async () => {
        try {
            setIsCapturing(true);
            const element = document.getElementById(props.targetElementId);
            if (!element) {
                console.error('Target element not found');
                return;
            }

            const dataUrl = await htmlToImage.toPng(element, {
                quality: 1,
                pixelRatio: scale(),
                backgroundColor: '#171717', // neutral-900
            });

            setPreviewUrl(dataUrl);
        } catch (error) {
            console.error('Error generating preview:', error);
        } finally {
            setIsCapturing(false);
        }
    };

    const handleSave = async () => {
        try {
            setIsCapturing(true);
            const element = document.getElementById(props.targetElementId);
            if (!element) {
                console.error('Target element not found');
                return;
            }

            const dataUrl = await htmlToImage.toPng(element, {
                quality: 1,
                pixelRatio: scale(),
                backgroundColor: '#171717', // neutral-900
            });

            // Convert data URL to blob and save
            const response = await fetch(dataUrl);
            const blob = await response.blob();

            const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
            saveAs(blob, `code-capture-${timestamp}.png`);

            props.onClose();
        } catch (error) {
            console.error('Error saving image:', error);
        } finally {
            setIsCapturing(false);
        }
    };

    const handleScaleChange = async (newScale: number) => {
        setScale(newScale);
        await generatePreview();
    };

    return (
        <Motion.div
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            transition={{ duration: 0.1, easing: "ease-out" }}
            class="h-screen w-screen fixed z-[100] bg-[#12121275] backdrop-blur-sm flex flex-col items-center justify-center"
        >
            <Motion.div
                initial={{ scale: 0.8, opacity: 0, y: 20 }}
                animate={{ scale: 1, opacity: 1, y: 0 }}
                exit={{ scale: 0.8, opacity: 0, y: 20 }}
                transition={{ duration: 0.4, easing: "ease-out", delay: 0.1 }}
                class="max-w-4xl max-h-[70vh] overflow-auto rounded-lg p-1 duration-200"
            >
                {/* Capture preview area */}
                {isCapturing() ? (
                    <Motion.div
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        class="flex items-center justify-center h-64 min-h-[400px]"
                    >
                        <div class="animate-spin rounded-full h-8 w-8 border-b-2 border-white"></div>
                        <span class="ml-2">Generating preview...</span>
                    </Motion.div>
                ) : previewUrl() ? (
                    <Motion.div
                        initial={{ scale: 0.9, opacity: 0 }}
                        animate={{ scale: 1, opacity: 1 }}
                        transition={{ duration: 0.1, easing: "ease-out" }}
                        class="relative p-0.5 rounded-lg"
                        style={{
                            background: 'conic-gradient(from 0deg, #7AB6FF, #A2E6B8, #FFD366, #FF9494, #DCA7FF, #7AB6FF)',
                            filter: 'drop-shadow(0 0 20px rgba(122, 182, 255, 0.5))'
                        }}
                    >
                        <div
                            class="max-w-full h-auto rounded border-0 block min-h-[400px] bg-cover bg-center bg-no-repeat"
                            style={{
                                'background-image': `url(${previewUrl()})`,
                                'width': '800px',
                                'aspect-ratio': 'auto',
                                'user-select': 'none',
                                'pointer-events': 'none',
                            }}
                            title="Code editor capture preview"
                        />
                    </Motion.div>
                ) : (
                    <div class="flex items-center justify-center h-64 text-neutral-400">
                        No preview available
                    </div>
                )}
            </Motion.div>

            <Motion.div
                initial={{ opacity: 0, y: 10 }}
                animate={{ opacity: 1, y: 0 }}
                transition={{ duration: 0.3, easing: "ease-out", delay: 0.2 }}
                class="pt-8 flex items-center justify-between space-x-8"
            >
                <Motion.button
                    whileHover={{ scale: 1.05 }}
                    whileTap={{ scale: 0.95 }}
                    onClick={handleSave}
                    disabled={isCapturing() || !previewUrl()}
                    class="flex items-center space-x-2 px-4 py-2 hover:opacity-40 disabled:bg-neutral-600 disabled:cursor-not-allowed rounded-md transition-colors"
                >
                    <ImageDown size={16} />
                    <span class="text-sm">{isCapturing() ? 'Capturing...' : 'Save Image'}</span>
                </Motion.button>

                <div class="flex items-center space-x-3">
                    <span class="text-sm text-neutral-300">Image size:</span>
                    <Motion.button
                        whileHover={{ scale: 1.1 }}
                        whileTap={{ scale: 0.9 }}
                        onClick={() => handleScaleChange(1)}
                        class={`text-sm px-2 py-1 rounded transition-colors ${scale() === 1 ? ' text-white' : 'text-white/40'
                            }`}
                    >
                        x1
                    </Motion.button>
                    <Motion.button
                        whileHover={{ scale: 1.1 }}
                        whileTap={{ scale: 0.9 }}
                        onClick={() => handleScaleChange(2)}
                        class={`text-sm px-2 py-1 rounded transition-colors ${scale() === 2 ? ' text-white' : 'text-white/40'
                            }`}
                    >
                        x2
                    </Motion.button>
                </div>
            </Motion.div>

            {/* Instructions */}
            <Motion.div
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                transition={{ duration: 0.3, easing: "ease-out", delay: 0.3 }}
                class="mt-4 text-xs text-neutral-400 text-center"
            >
                Press <kbd class="px-1 py-0.5 bg-neutral-700 rounded text-xs">Ctrl+Alt+F</kbd> to capture • <kbd class="px-1 py-0.5 bg-neutral-700 rounded text-xs">Esc</kbd> to close
            </Motion.div>
        </Motion.div>
    );
}

export default Capture;