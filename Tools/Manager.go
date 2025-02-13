package Manager

import (
	"Camouflage/Packages/Colors"
	"Camouflage/Packages/Utils"
	"bytes"
	"debug/pe"
	"encoding/binary"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"
	"unicode/utf16"
)

// VersionInfo holds both fixed and string file info
type VersionInfo struct {
	FileVersion    [4]uint16 // Major1, Minor1, Major2, Minor2
	ProductVersion [4]uint16 // Major1, Minor1, Major2, Minor2
	FileFlagsMask  uint32
	FileFlags      uint32
	FileOS         uint32
	FileType       uint32
	FileSubtype    uint32
	StringValues   map[string]string
}

func FindVersionInfo(filePath string) (*VersionInfo, error) {
	info := &VersionInfo{
		StringValues: make(map[string]string),
	}

	// Open and parse PE file
	f, err := os.Open(filePath)
	if err != nil {
		return nil, fmt.Errorf("error opening file: %v", err)
	}
	defer f.Close()

	pefile, err := pe.NewFile(f)
	if err != nil {
		return nil, fmt.Errorf("error parsing PE file: %v", err)
	}
	defer pefile.Close()

	// Find the resource directory
	var virtualAddr uint32
	var size uint32

	switch oh := pefile.OptionalHeader.(type) {
	case *pe.OptionalHeader32:
		if len(oh.DataDirectory) > pe.IMAGE_DIRECTORY_ENTRY_RESOURCE {
			virtualAddr = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress
			size = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].Size
		}
	case *pe.OptionalHeader64:
		if len(oh.DataDirectory) > pe.IMAGE_DIRECTORY_ENTRY_RESOURCE {
			virtualAddr = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress
			size = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].Size
		}
	}

	if virtualAddr == 0 || size == 0 {
		return nil, fmt.Errorf("resource directory not found")
	}

	// Find section containing version info
	var resourceSection *pe.Section
	for _, section := range pefile.Sections {
		if section.VirtualAddress <= virtualAddr && virtualAddr < section.VirtualAddress+section.Size {
			resourceSection = section
			break
		}
	}

	if resourceSection == nil {
		return nil, fmt.Errorf("version info section not found")
	}

	data, err := resourceSection.Data()
	if err != nil {
		return nil, fmt.Errorf("error reading resource section: %v", err)
	}

	// Find VS_VERSION_INFO structure
	var versionStart int = -1
	pattern := []byte{'V', 0x00, 'S', 0x00, '_', 0x00, 'V', 0x00, 'E', 0x00, 'R', 0x00, 'S', 0x00, 'I', 0x00, 'O', 0x00, 'N', 0x00, '_', 0x00, 'I', 0x00, 'N', 0x00, 'F', 0x00, 'O', 0x00}

	for i := 0; i < len(data)-len(pattern); i++ {
		matched := true
		for j := 0; j < len(pattern); j++ {
			if data[i+j] != pattern[j] {
				matched = false
				break
			}
		}
		if matched {
			versionStart = i
			break
		}
	}

	if versionStart == -1 {
		return nil, fmt.Errorf("VS_VERSION_INFO not found")
	}

	// Skip over the VS_VERSION_INFO header (length + valLength + type)
	offset := versionStart + 32 + 6 // Skip string + padding to reach fixed file info

	// Read VS_FIXEDFILEINFO
	if offset+52 <= len(data) { // Fixed file info is 52 bytes
		info.FileVersion[0] = binary.LittleEndian.Uint16(data[offset+10:])
		info.FileVersion[1] = binary.LittleEndian.Uint16(data[offset+8:])
		info.FileVersion[2] = binary.LittleEndian.Uint16(data[offset+14:])
		info.FileVersion[3] = binary.LittleEndian.Uint16(data[offset+12:])

		info.ProductVersion[0] = binary.LittleEndian.Uint16(data[offset+18:])
		info.ProductVersion[1] = binary.LittleEndian.Uint16(data[offset+16:])
		info.ProductVersion[2] = binary.LittleEndian.Uint16(data[offset+22:])
		info.ProductVersion[3] = binary.LittleEndian.Uint16(data[offset+20:])

		info.FileFlagsMask = binary.LittleEndian.Uint32(data[offset+24:])
		info.FileFlags = binary.LittleEndian.Uint32(data[offset+28:])
		info.FileOS = binary.LittleEndian.Uint32(data[offset+32:])
		info.FileType = binary.LittleEndian.Uint32(data[offset+36:])
		info.FileSubtype = binary.LittleEndian.Uint32(data[offset+40:])
	}

	// Look for StringFileInfo block
	stringKeys := []string{
		"CompanyName",
		"FileDescription",
		"FileVersion",
		"InternalName",
		"LegalCopyright",
		"OriginalFilename",
		"ProductName",
		"ProductVersion",
	}

	// Search for each string value
	for _, key := range stringKeys {
		pattern := make([]byte, len(key)*2)
		for i, c := range key {
			pattern[i*2] = byte(c)
			pattern[i*2+1] = 0x00
		}

		for i := offset; i < len(data)-len(pattern); i++ {
			matched := true
			for j := 0; j < len(pattern); j++ {
				if data[i+j] != pattern[j] {
					matched = false
					break
				}
			}
			if matched {
				// Found key, extract value
				valueStart := i + len(pattern)
				for valueStart < len(data)-1 {
					if data[valueStart] == 0 && data[valueStart+1] == 0 {
						valueStart += 2
						break
					}
					valueStart++
				}

				valueEnd := valueStart
				nullCount := 0
				for valueEnd < len(data)-1 {
					if data[valueEnd] == 0 && data[valueEnd+1] == 0 {
						nullCount++
						if nullCount >= 2 {
							break
						}
					} else {
						nullCount = 0
					}
					valueEnd++
				}

				if valueEnd > valueStart {
					chars := make([]uint16, 0)
					for j := valueStart; j < valueEnd; j += 2 {
						if j+1 < len(data) {
							chars = append(chars, uint16(data[j])|uint16(data[j+1])<<8)
						}
					}
					value := strings.TrimSpace(string(utf16.Decode(chars)))
					if value != "" {
						info.StringValues[key] = value
					}
				}
				break
			}
		}
	}

	return info, nil
}

// UpdateBinaryMetadata function
func UpdateBinaryMetadata(targetPath string, metadata map[string]string) error {
	// Read the entire file
	fileData, err := os.ReadFile(targetPath)
	if err != nil {
		return fmt.Errorf("error reading target file: %v", err)
	}

	// Parse the PE file
	pefile, err := pe.NewFile(bytes.NewReader(fileData))
	if err != nil {
		return fmt.Errorf("error parsing PE file: %v", err)
	}
	defer pefile.Close()

	// Find the resource directory
	var virtualAddr uint32
	var size uint32
	switch oh := pefile.OptionalHeader.(type) {
	case *pe.OptionalHeader32:
		if len(oh.DataDirectory) > pe.IMAGE_DIRECTORY_ENTRY_RESOURCE {
			virtualAddr = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress
			size = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].Size
		}
	case *pe.OptionalHeader64:
		if len(oh.DataDirectory) > pe.IMAGE_DIRECTORY_ENTRY_RESOURCE {
			virtualAddr = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress
			size = oh.DataDirectory[pe.IMAGE_DIRECTORY_ENTRY_RESOURCE].Size
		}
	}

	// Check both virtualAddr and size for validity
	if virtualAddr == 0 || size == 0 {
		return fmt.Errorf("resource directory not found or empty: addr=%d, size=%d", virtualAddr, size)
	}

	// Find resource section
	var resourceSection *pe.Section
	for _, section := range pefile.Sections {
		if section.VirtualAddress <= virtualAddr && virtualAddr < section.VirtualAddress+section.Size {
			resourceSection = section
			break
		}
	}
	if resourceSection == nil {
		return fmt.Errorf("resource section not found")
	}

	// Get the resource data
	data, err := resourceSection.Data()
	if err != nil {
		return fmt.Errorf("error reading resource section: %v", err)
	}

	// Find the StringFileInfo block
	var stringInfoStart int = -1
	for i := 0; i < len(data)-2; i++ {
		if data[i] == 'S' && data[i+2] == 't' {
			stringInfoStart = i
			break
		}
	}
	if stringInfoStart == -1 {
		return fmt.Errorf("StringFileInfo block not found")
	}

	// Create a copy of the file data
	newFileData := make([]byte, len(fileData))
	copy(newFileData, fileData)

	// Calculate the offset in the file for the resource section
	fileOffset := resourceSection.Offset

	// For each metadata key-value pair
	for key, value := range metadata {
		// Create UTF-16 pattern to search for
		pattern := make([]byte, len(key)*2)
		for i, c := range key {
			pattern[i*2] = byte(c)
			pattern[i*2+1] = 0x00
		}

		// Find the key in the resource section
		for i := stringInfoStart; i < len(data)-len(pattern); i++ {
			matched := true
			for j := 0; j < len(pattern); j++ {
				if data[i+j] != pattern[j] {
					matched = false
					break
				}
			}

			if matched {
				// Convert new value to UTF-16
				valueUtf16 := utf16.Encode([]rune(value))
				valueBytes := make([]byte, len(valueUtf16)*2)
				for j, c := range valueUtf16 {
					valueBytes[j*2] = byte(c)
					valueBytes[j*2+1] = byte(c >> 8)
				}

				// Find the start of the old value
				valueStart := i + len(pattern)
				for valueStart < len(data)-1 {
					if valueStart+2 < len(data) && data[valueStart] == 0 && data[valueStart+1] == 0 {
						valueStart += 2
						break
					}
					valueStart++
				}

				// Find the end of the old value
				valueEnd := valueStart
				nullCount := 0
				for valueEnd < len(data)-1 {
					if data[valueEnd] == 0 && data[valueEnd+1] == 0 {
						nullCount++
						if nullCount >= 2 {
							break
						}
					} else {
						nullCount = 0
					}
					valueEnd++
				}

				// Calculate absolute positions in file
				absStart := fileOffset + uint32(valueStart)
				absEnd := fileOffset + uint32(valueEnd)

				// Write new value to the file data
				// Ensure we don't write beyond the original value's space
				maxLen := absEnd - absStart
				if uint32(len(valueBytes)) > maxLen {
					valueBytes = valueBytes[:maxLen]
				}
				copy(newFileData[absStart:absEnd], valueBytes)

				// Pad with nulls if needed
				for j := len(valueBytes); j < int(maxLen); j++ {
					newFileData[absStart+uint32(j)] = 0
				}

				break
			}
		}
	}

	// Create new filename with timestamp
	dir := filepath.Dir(targetPath)
	originalFileName := filepath.Base(targetPath)
	fileExt := filepath.Ext(originalFileName)
	nameWithoutExt := strings.TrimSuffix(originalFileName, fileExt)

	// Create timestamp
	timestamp := time.Now().Format("20060102_150405")

	// Create new filename with timestamp
	newFileName := fmt.Sprintf("%s_%s%s", nameWithoutExt, timestamp, fileExt)
	newFilePath := filepath.Join(dir, newFileName)

	// Write the modified data back to the file
	err = os.WriteFile(newFilePath, newFileData, 0644)
	if err != nil {
		return fmt.Errorf("error writing updated file: %v", err)
	}

	// Call function named GetAbsolutePath
	absNewFilePath, err := Utils.GetAbsolutePath(newFilePath)
	if err != nil {
		return fmt.Errorf("error getting absolute path: %v", err)
	}

	fmt.Printf("[+] Modified binary saved to: %s\n\n", Colors.BoldGreen(absNewFilePath))

	return nil
}
