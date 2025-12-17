#!/bin/sh

cleanup() {
    rc=$?
    if [ -n "$temp" ] && [ -d "$temp" ]; then
        rm -rf "$temp"
    fi
    exit $rc
}

file="$1"
outputDir=$(dirname "$file")
outputLineRegex='\$output="\K.*(?=")'

if [ ! -f "$file" ]; then
    echo "File not found"
    exit 1
fi

outputFile=$(grep -oP $outputLineRegex "$file")
if [ ! "$outputFile" ]; then
    echo "output path not specified"
    exit 1
fi
outputFileName=$(basename "$outputFile")

temp=$(mktemp -d)

buildC() {
    gcc "$file" -o "$temp/$outputFileName"
    cp "$temp/$outputFileName" "$outputDir/$outputFile"
}
buildCpp() {
    g++ "$file" -o "$temp/$outputFileName"
    cp "$temp/$outputFileName" "$outputDir/$outputFile"
}
buildTex() {
    fileName=$(basename "$file" .tex)
    echo $temp
    latex -output-directory "$temp" "$file"
    dvips -o "$temp/$fileName.ps" "$temp/$fileName.dvi"
    ps2pdf "$temp/$fileName.ps" "$temp/$fileName.pdf"
    cp "$temp/$fileName.pdf" "$outputDir/$outputFile.pdf"
    rm -rf "$temp"
}

trap cleanup EXIT HUP INT QUIT TERM
case "$file" in
    *.c) buildC;;
    *.cpp) buildCpp;;
    *.tex) buildTex;;
    *) echo "Unsupported file type";;
esac

echo "$outputDir/$outputFile"
