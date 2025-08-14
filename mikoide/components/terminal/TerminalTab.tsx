import { Terminal, X, Play, AlertCircle } from "lucide-solid";

interface TerminalTabProps {
    id: string;
    name: string;
    status: 'idle' | 'running' | 'error';
    isActive: boolean;
    onSelect: (id: string) => void;
    onClose: (id: string) => void;
    canClose: boolean;
}

function TerminalTab(props: TerminalTabProps) {
    const getStatusColor = () => {
        switch (props.status) {
            case 'running': return 'text-yellow-400';
            case 'error': return 'text-red-400';
            default: return 'text-green-400';
        }
    };
    
    const getStatusIcon = () => {
        switch (props.status) {
            case 'running': return <Play size={12} />;
            case 'error': return <AlertCircle size={12} />;
            default: return <Terminal size={12} />;
        }
    };
    
    return (
        <div 
            class={`flex items-center gap-2 px-3 py-1.5 rounded-t text-xs cursor-pointer transition-all duration-200 min-w-0 group ${
                props.isActive 
                    ? 'bg-neutral-900 text-white border-b-2 border-blue-400 shadow-lg' 
                    : 'text-gray-400 hover:text-white hover:bg-neutral-700'
            }`}
            onClick={() => props.onSelect(props.id)}
        >
            {/* Status Icon */}
            <div class={`flex items-center gap-1 transition-colors ${getStatusColor()}`}>
                {getStatusIcon()}
            </div>
            
            {/* Terminal Name */}
            <span class="truncate max-w-24 font-medium">{props.name}</span>
            
            {/* Close Button */}
            {props.canClose && (
                <button 
                    class="opacity-0 group-hover:opacity-100 text-gray-500 hover:text-red-400 transition-all duration-200 p-0.5 rounded hover:bg-red-400/10"
                    onClick={(e) => {
                        e.stopPropagation();
                        props.onClose(props.id);
                    }}
                    title="Close Terminal"
                >
                    <X size={12} />
                </button>
            )}
            
            {/* Active Indicator */}
            {props.isActive && (
                <div class="absolute bottom-0 left-0 right-0 h-0.5 bg-blue-400 rounded-t"></div>
            )}
        </div>
    );
}

export default TerminalTab;