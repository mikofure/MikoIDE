import { createSignal, Show, onMount, onCleanup } from "solid-js";
import { Portal } from "solid-js/web";

export interface DialogProps {
    isOpen: boolean;
    title: string;
    message: string;
    type?: 'confirm' | 'input' | 'alert';
    inputPlaceholder?: string;
    confirmText?: string;
    cancelText?: string;
    onConfirm?: (value?: string) => void;
    onCancel?: () => void;
    onClose?: () => void;
}

function Dialog(props: DialogProps) {
    const [inputValue, setInputValue] = createSignal("");
    let inputRef: HTMLInputElement | undefined;
    let dialogRef: HTMLDivElement | undefined;

    const handleConfirm = () => {
        const value = props.type === 'input' ? inputValue() : 'confirmed';
        props.onConfirm?.(value);
        props.onClose?.();
    };

    const handleCancel = () => {
        props.onCancel?.();
        props.onClose?.();
    };

    const handleKeyDown = (e: KeyboardEvent) => {
        if (e.key === 'Enter' && props.isOpen) {
            e.preventDefault();
            handleConfirm();
        } else if (e.key === 'Escape' && props.isOpen) {
            e.preventDefault();
            handleCancel();
        }
    };

    const handleBackdropClick = (e: MouseEvent) => {
        if (e.target === dialogRef) {
            handleCancel();
        }
    };

    onMount(() => {
        document.addEventListener('keydown', handleKeyDown);
    });

    onCleanup(() => {
        document.removeEventListener('keydown', handleKeyDown);
    });

    return (
        <Show when={props.isOpen}>
            <Portal>
                <div
                    ref={dialogRef}
                    class="fixed inset-0 bg-black/50 flex items-center justify-center z-[9999]"
                    onClick={handleBackdropClick}
                >
                    <div class="bg-[#1a1a1a] border border-[#323132] rounded-lg p-6 min-w-96 max-w-md">
                        <h3 class="text-white text-lg font-semibold mb-4">{props.title}</h3>
                        <p class="text-gray-300 text-sm mb-4">{props.message}</p>
                        
                        <Show when={props.type === 'input'}>
                            <input
                                ref={inputRef}
                                type="text"
                                placeholder={props.inputPlaceholder || ""}
                                value={inputValue()}
                                onInput={(e) => setInputValue(e.currentTarget.value)}
                                class="w-full bg-[#2d2d30] border border-[#464647] rounded px-3 py-2 text-white text-sm mb-4 focus:outline-none focus:border-[#4fc3f7]"
                                autofocus
                            />
                        </Show>
                        
                        <div class="flex gap-2 justify-end">
                            <Show when={props.type !== 'alert'}>
                                <button
                                    onClick={handleCancel}
                                    class="px-4 py-2 bg-[#2d2d30] hover:bg-[#3e3e42] text-gray-300 rounded text-sm transition-colors"
                                >
                                    {props.cancelText || 'Cancel'}
                                </button>
                            </Show>
                            <button
                                onClick={handleConfirm}
                                class="px-4 py-2 bg-[#4fc3f7] hover:bg-[#29b6f6] text-white rounded text-sm transition-colors"
                            >
                                {props.confirmText || (props.type === 'input' ? 'Execute' : 'OK')}
                            </button>
                        </div>
                    </div>
                </div>
            </Portal>
        </Show>
    );
}

export default Dialog;