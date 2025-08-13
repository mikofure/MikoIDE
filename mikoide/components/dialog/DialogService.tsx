import { createSignal } from "solid-js";
import Dialog from "./Dialog";
import type { DialogProps } from "./Dialog";

interface DialogState extends Omit<DialogProps, 'isOpen' | 'onConfirm' | 'onCancel' | 'onClose'> {
    resolve?: (value: string | null) => void;
}

const [dialogState, setDialogState] = createSignal<DialogState | null>(null);
const [isDialogOpen, setIsDialogOpen] = createSignal(false);

export const showDialog = (title: string, message: string, inputPlaceholder?: string): Promise<string | null> => {
    return new Promise<string | null>((resolve) => {
        setDialogState({
            title,
            message,
            type: inputPlaceholder ? 'input' : 'confirm',
            inputPlaceholder,
            resolve
        });
        setIsDialogOpen(true);
    });
};

export const showAlert = (title: string, message: string): Promise<string | null> => {
    return new Promise<string | null>((resolve) => {
        setDialogState({
            title,
            message,
            type: 'alert',
            resolve
        });
        setIsDialogOpen(true);
    });
};

export const showConfirm = (title: string, message: string, confirmText?: string, cancelText?: string): Promise<string | null> => {
    return new Promise<string | null>((resolve) => {
        setDialogState({
            title,
            message,
            type: 'confirm',
            confirmText,
            cancelText,
            resolve
        });
        setIsDialogOpen(true);
    });
};

export const showInput = (title: string, message: string, placeholder?: string, confirmText?: string): Promise<string | null> => {
    return new Promise<string | null>((resolve) => {
        setDialogState({
            title,
            message,
            type: 'input',
            inputPlaceholder: placeholder,
            confirmText,
            resolve
        });
        setIsDialogOpen(true);
    });
};

export const showTrustDialog = (folderPath: string): Promise<boolean> => {
    return new Promise<boolean>((resolve) => {
        setDialogState({
            title: "Trust Folder",
            message: `Do you trust the authors of the files in this folder?\n\nFolder: ${folderPath}\n\nMalicious files can harm your computer. Only trust this folder if you trust its source.`,
            type: 'confirm',
            confirmText: 'Yes, I trust the authors',
            cancelText: 'No, I don\'t trust the authors',
            resolve: (value) => resolve(value !== null)
        });
        setIsDialogOpen(true);
    });
};

const handleConfirm = (value?: string) => {
    const state = dialogState();
    if (state?.resolve) {
        state.resolve(value || 'confirmed');
    }
    closeDialog();
};

const handleCancel = () => {
    const state = dialogState();
    if (state?.resolve) {
        state.resolve(null);
    }
    closeDialog();
};

const closeDialog = () => {
    setIsDialogOpen(false);
    setDialogState(null);
};

export function DialogProvider() {
    const state = dialogState();
    
    return (
        <Dialog
            isOpen={isDialogOpen()}
            title={state?.title || ""}
            message={state?.message || ""}
            type={state?.type || 'confirm'}
            inputPlaceholder={state?.inputPlaceholder}
            confirmText={state?.confirmText}
            cancelText={state?.cancelText}
            onConfirm={handleConfirm}
            onCancel={handleCancel}
            onClose={closeDialog}
        />
    );
}