#!/usr/bin/env python3
"""
Theme Preview Application
A Qt6-based application to preview YAML theme files with color swatches and syntax highlighting.
"""

import sys
import os
import yaml
from pathlib import Path
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, 
    QLabel, QComboBox, QScrollArea, QFrame, QGridLayout, QTextEdit,
    QSplitter, QGroupBox, QPushButton, QMessageBox
)
from PySide6.QtCore import Qt, QSize
from PySide6.QtGui import QColor, QPalette, QFont, QPixmap, QPainter


class ColorSwatch(QFrame):
    """A widget that displays a color swatch with hex value."""
    
    def __init__(self, color_hex, label_text, parent=None):
        super().__init__(parent)
        
        # Ensure color_hex is a string
        if isinstance(color_hex, dict):
            # If it's a dict, try to get a color value or use a default
            color_hex = color_hex.get('color', color_hex.get('value', '#FFFFFF'))
        elif not isinstance(color_hex, str):
            # Convert to string if it's not already
            color_hex = str(color_hex)
            
        self.color_hex = color_hex
        self.label_text = label_text
        self.setFixedSize(120, 80)
        self.setFrameStyle(QFrame.Box)
        self.setLineWidth(1)
        
        # Set background color
        self.setStyleSheet(f"""
            ColorSwatch {{
                background-color: {color_hex};
                border: 2px solid #333;
                border-radius: 8px;
            }}
        """)
        
        # Add label
        layout = QVBoxLayout(self)
        layout.setContentsMargins(5, 5, 5, 5)
        
        name_label = QLabel(label_text)
        name_label.setAlignment(Qt.AlignCenter)
        name_label.setStyleSheet("""
            QLabel {
                background-color: rgba(0, 0, 0, 0.7);
                color: white;
                padding: 2px;
                border-radius: 3px;
                font-size: 10px;
                font-weight: bold;
            }
        """)
        
        hex_label = QLabel(color_hex.upper())
        hex_label.setAlignment(Qt.AlignCenter)
        hex_label.setStyleSheet("""
            QLabel {
                background-color: rgba(0, 0, 0, 0.7);
                color: white;
                padding: 2px;
                border-radius: 3px;
                font-size: 9px;
            }
        """)
        
        layout.addWidget(name_label)
        layout.addStretch()
        layout.addWidget(hex_label)


class SyntaxPreview(QTextEdit):
    """A text editor that shows syntax highlighting preview."""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setReadOnly(True)
        self.setFixedHeight(300)
        
        # Sample code for syntax highlighting
        sample_code = '''// Sample C++ Code
#include <iostream>
#include <string>

class HelloWorld {
private:
    std::string message;
    int count = 42;
    
public:
    HelloWorld(const std::string& msg) : message(msg) {}
    
    void display() {
        // Print the message
        std::cout << "Message: " << message << std::endl;
        std::cout << "Count: " << count << std::endl;
    }
};

int main() {
    HelloWorld hello("Welcome to Theme Preview!");
    hello.display();
    return 0;
}'''
        
        self.setPlainText(sample_code)
        
    def apply_theme_colors(self, theme_data):
        """Apply theme colors to the syntax preview."""
        if 'syntax' not in theme_data or 'palette' not in theme_data:
            return
            
        syntax = theme_data['syntax']
        palette = theme_data['palette']
        
        # Set background and foreground
        bg_color = palette.get('background', '#000000')
        fg_color = palette.get('foreground', '#ffffff')
        
        style = f"""
            QTextEdit {{
                background-color: {bg_color};
                color: {fg_color};
                border: 1px solid #ccc;
                border-radius: 4px;
                padding: 8px;
                font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
                font-size: 12px;
                line-height: 1.4;
            }}
        """
        
        self.setStyleSheet(style)


class ThemePreviewWidget(QWidget):
    """Main widget for displaying theme preview."""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.current_theme = None
        self.setup_ui()
        
    def setup_ui(self):
        """Setup the user interface."""
        layout = QVBoxLayout(self)
        
        # Theme selector
        selector_layout = QHBoxLayout()
        selector_layout.addWidget(QLabel("Select Theme:"))
        
        self.theme_combo = QComboBox()
        self.theme_combo.currentTextChanged.connect(self.load_theme)
        selector_layout.addWidget(self.theme_combo)
        
        refresh_btn = QPushButton("Refresh")
        refresh_btn.clicked.connect(self.refresh_themes)
        selector_layout.addWidget(refresh_btn)
        
        selector_layout.addStretch()
        layout.addLayout(selector_layout)
        
        # Main content area
        splitter = QSplitter(Qt.Horizontal)
        
        # Left panel - Color swatches
        left_panel = self.create_color_panel()
        splitter.addWidget(left_panel)
        
        # Right panel - Syntax preview
        right_panel = self.create_syntax_panel()
        splitter.addWidget(right_panel)
        
        splitter.setSizes([400, 600])
        layout.addWidget(splitter)
        
        # Load available themes
        self.refresh_themes()
        
    def create_color_panel(self):
        """Create the color swatches panel."""
        scroll_area = QScrollArea()
        scroll_widget = QWidget()
        self.color_layout = QVBoxLayout(scroll_widget)
        
        # Palette colors group
        self.palette_group = QGroupBox("Palette Colors")
        self.palette_layout = QGridLayout(self.palette_group)
        self.color_layout.addWidget(self.palette_group)
        
        # Syntax colors group
        self.syntax_group = QGroupBox("Syntax Colors")
        self.syntax_layout = QGridLayout(self.syntax_group)
        self.color_layout.addWidget(self.syntax_group)
        
        # UI colors group
        self.ui_group = QGroupBox("UI Colors")
        self.ui_layout = QGridLayout(self.ui_group)
        self.color_layout.addWidget(self.ui_group)
        
        self.color_layout.addStretch()
        scroll_area.setWidget(scroll_widget)
        scroll_area.setWidgetResizable(True)
        scroll_area.setMinimumWidth(420)
        
        return scroll_area
        
    def create_syntax_panel(self):
        """Create the syntax preview panel."""
        panel = QWidget()
        layout = QVBoxLayout(panel)
        
        layout.addWidget(QLabel("Syntax Highlighting Preview:"))
        
        self.syntax_preview = SyntaxPreview()
        layout.addWidget(self.syntax_preview)
        
        # Theme info
        self.theme_info = QLabel("No theme selected")
        self.theme_info.setStyleSheet("""
            QLabel {
                background-color: #f0f0f0;
                padding: 8px;
                border-radius: 4px;
                border: 1px solid #ccc;
            }
        """)
        layout.addWidget(self.theme_info)
        
        return panel
        
    def refresh_themes(self):
        """Refresh the list of available themes."""
        self.theme_combo.clear()
        
        # Look for theme files in the shared/theme directory
        theme_dir = Path(__file__).parent.parent.parent / "shared" / "theme"
        
        if theme_dir.exists():
            theme_files = list(theme_dir.glob("*.yml"))
            theme_names = [f.stem for f in theme_files]
            theme_names.sort()
            
            self.theme_combo.addItems(theme_names)
            
            if theme_names:
                self.load_theme(theme_names[0])
        else:
            QMessageBox.warning(self, "Warning", f"Theme directory not found: {theme_dir}")
            
    def load_theme(self, theme_name):
        """Load and display a theme."""
        if not theme_name:
            return
            
        theme_dir = Path(__file__).parent.parent.parent / "shared" / "theme"
        theme_file = theme_dir / f"{theme_name}.yml"
        
        try:
            with open(theme_file, 'r', encoding='utf-8') as f:
                theme_data = yaml.safe_load(f)
                
            self.current_theme = theme_data
            self.display_theme(theme_data)
            
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to load theme '{theme_name}':\n{str(e)}")
            
    def display_theme(self, theme_data):
        """Display the theme colors and information."""
        # Clear existing widgets
        self.clear_layout(self.palette_layout)
        self.clear_layout(self.syntax_layout)
        self.clear_layout(self.ui_layout)
        
        # Display palette colors
        if 'palette' in theme_data:
            palette = theme_data['palette']
            row, col = 0, 0
            for key, color in palette.items():
                swatch = ColorSwatch(color, key.title())
                self.palette_layout.addWidget(swatch, row, col)
                col += 1
                if col >= 3:
                    col = 0
                    row += 1
                    
        # Display syntax colors
        if 'syntax' in theme_data:
            syntax = theme_data['syntax']
            row, col = 0, 0
            for key, color in syntax.items():
                swatch = ColorSwatch(color, key.title())
                self.syntax_layout.addWidget(swatch, row, col)
                col += 1
                if col >= 3:
                    col = 0
                    row += 1
                    
        # Display UI colors (flattened)
        if 'ui' in theme_data:
            ui = theme_data['ui']
            row, col = 0, 0
            for section, colors in ui.items():
                if isinstance(colors, dict):
                    for key, color in colors.items():
                        label = f"{section}.{key}"
                        swatch = ColorSwatch(color, label)
                        self.ui_layout.addWidget(swatch, row, col)
                        col += 1
                        if col >= 3:
                            col = 0
                            row += 1
                            
        # Update syntax preview
        self.syntax_preview.apply_theme_colors(theme_data)
        
        # Update theme info
        theme_name = theme_data.get('name', 'Unknown Theme')
        theme_type = theme_data.get('type', 'Unknown Type')
        info_text = f"<b>{theme_name}</b><br>Type: {theme_type.title()}"
        self.theme_info.setText(info_text)
        
    def clear_layout(self, layout):
        """Clear all widgets from a layout."""
        while layout.count():
            child = layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()


class ThemePreviewApp(QMainWindow):
    """Main application window."""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Theme Preview - MikoIDE")
        self.setMinimumSize(1000, 700)
        
        # Set application icon (if available)
        self.setup_ui()
        
    def setup_ui(self):
        """Setup the main window UI."""
        central_widget = ThemePreviewWidget()
        self.setCentralWidget(central_widget)
        
        # Set window style
        self.setStyleSheet("""
            QMainWindow {
                background-color: #0e0e0e;
            }
            QGroupBox {
                font-weight: bold;
                border: 2px solid #cccccc;
                border-radius: 8px;
                margin-top: 1ex;
                padding-top: 10px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
            }
            QComboBox {
                padding: 5px;
                border: 1px solid #ccc;
                border-radius: 4px;
                min-width: 150px;
            }
            QPushButton {
                padding: 5px 15px;
                border: 1px solid #ccc;
                border-radius: 4px;
                background-color: #141414;
            }
            QPushButton:hover {
                background-color: #e6e6e6;
            }
        """)


def main():
    """Main application entry point."""
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    app.setApplicationName("Theme Preview")
    app.setApplicationVersion("1.0")
    
    # Check if required modules are available
    try:
        import yaml
        from PySide6 import QtWidgets
    except ImportError as e:
        print(f"Error: Required module not found: {e}")
        print("Please install required dependencies:")
        print("  pip install PySide6 PyYAML")
        return 1
    
    window = ThemePreviewApp()
    window.show()
    
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())