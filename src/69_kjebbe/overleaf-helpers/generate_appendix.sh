#!/bin/bash

# LaTeX Appendix Generator
# Generates a complete LaTeX appendix file with minted code listings
# Usage: ./generate_appendix.sh file_list.txt > output.tex

# Check arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <file_list.txt>" >&2
    echo "Example: $0 files_to_include.txt > appendix.tex" >&2
    exit 1
fi

FILE_LIST="$1"

# Check if file list exists
if [ ! -f "$FILE_LIST" ]; then
    echo "Error: File list '$FILE_LIST' not found" >&2
    exit 1
fi

# Function to detect language from filename
detect_language() {
    local filename="$1"
    local ext="${filename##*.}"
    ext=$(echo "$ext" | tr '[:upper:]' '[:lower:]')
    
    case "$ext" in
        c|h)
            echo "c"
            ;;
        cpp|cxx|cc|hpp|hxx)
            echo "cpp"
            ;;
        asm|s)
            echo "nasm"
            ;;
        py)
            echo "python"
            ;;
        sh)
            echo "bash"
            ;;
        js)
            echo "javascript"
            ;;
        java)
            echo "java"
            ;;
        rs)
            echo "rust"
            ;;
        go)
            echo "go"
            ;;
        *)
            echo "text"
            ;;
    esac
}

# Function to generate safe label from filename
generate_label() {
    local filename="$1"
    # Remove path, replace special chars with underscores
    local base=$(basename "$filename")
    local label=$(echo "$base" | sed 's/[^a-zA-Z0-9]/_/g' | tr '[:upper:]' '[:lower:]')
    echo "lst:$label"
}

# Output LaTeX header
cat << 'EOF'
\documentclass[../main.tex]{subfiles}
\begin{document}

\section{Appendix}
\label{sect:appendix}

EOF

# Process file list line by line
while IFS= read -r line || [[ -n "$line" ]]; do
    # Skip empty lines
    [[ -z "$line" ]] && continue
    
    # Skip comment lines (lines starting with //)
    [[ "$line" =~ ^[[:space:]]*// ]] && continue
    
    # Check for markdown headers
    if [[ "$line" =~ ^###[[:space:]]+(.*)$ ]]; then
        # Subsubsection
        title="${BASH_REMATCH[1]}"
        # Generate safe label
        label=$(echo "$title" | sed 's/[^a-zA-Z0-9]/_/g' | tr '[:upper:]' '[:lower:]')
        echo ""
        echo "\subsubsection{$title}"
        echo "\label{apx:$label}"
        echo ""
        
    elif [[ "$line" =~ ^##[[:space:]]+(.*)$ ]]; then
        # Subsection
        title="${BASH_REMATCH[1]}"
        label=$(echo "$title" | sed 's/[^a-zA-Z0-9]/_/g' | tr '[:upper:]' '[:lower:]')
        echo ""
        echo "\subsection{$title}"
        echo "\label{apx:$label}"
        echo ""
        
    elif [[ "$line" =~ ^#[[:space:]]+(.*)$ ]]; then
        # Section
        title="${BASH_REMATCH[1]}"
        label=$(echo "$title" | sed 's/[^a-zA-Z0-9]/_/g' | tr '[:upper:]' '[:lower:]')
        echo ""
        echo "\section{$title}"
        echo "\label{apx:$label}"
        echo ""
        
    else
        # Regular line - treat as file path
        filepath=$(echo "$line" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
        
        # Check if file exists
        if [ ! -f "$filepath" ]; then
            echo "% WARNING: File not found: $filepath" >&2
            echo "% Skipping: $filepath"
            continue
        fi
        
        # Get file info
        filename=$(basename "$filepath")
        language=$(detect_language "$filename")
        label=$(generate_label "$filename")
        
        # Escape special LaTeX characters in caption
        escaped_filename=$(echo "$filename" | sed 's/_/\\_/g;s/&/\\&/g;s/%/\\%/g;s/\\$/\\$/g')
        
        # Output listing using heredoc for cleaner template
        cat << EOF

\begin{listing}[H]
    \begin{minted}[
        linenos,
        breaklines,
        frame=single,
        framesep=3mm,
        baselinestretch=1.1,
        numbersep=6pt,
        fontsize=\footnotesize,
        tabsize=4,
        bgcolor=white,
        style=friendly,
    ]{$language}
EOF
        
        # Read and output file content (escape backslashes for LaTeX)
        sed 's/\\/\\\\/g' "$filepath"
        
        cat << EOF
    \end{minted}
    \caption{Implementation of \texttt{$escaped_filename}}
    \label{$label}
\end{listing}

EOF
    fi
done < "$FILE_LIST"

# Output LaTeX footer
cat << 'EOF'
\end{document}
EOF
