package Arguments

import (
	"Camouflage/Packages/Colors"
	"Camouflage/Packages/Manager"
	"Camouflage/Packages/Output"
	"Camouflage/Packages/Protection"
	"bytes"
	"debug/pe"
	"encoding/binary"
	"fmt"
	"log"
	"os"
	"time"

	"github.com/spf13/cobra"
)

// infoArgument represents the 'info' command in the CLI.
var infoArgument = &cobra.Command{
	// Use defines how the command should be called.
	Use:          "info",
	Short:        "Show metadata information about the binary",
	SilenceUsage: true,
	Aliases:      []string{"Info", "INFO"},

	// RunE defines the function to run when the command is executed.
	RunE: func(cmd *cobra.Command, args []string) error {
		logger := log.New(os.Stderr, "[!] ", 0)

		// Show ASCII banner
		ShowAscii()

		// Check if additional arguments were provided
		if len(os.Args) <= 2 {
			err := cmd.Help()
			if err != nil {
				logger.Fatal("Error ", err)
				return err
			}
			os.Exit(0)
		}

		// Parse the flags
		file, _ := cmd.Flags().GetString("file")
		output, _ := cmd.Flags().GetString("output")

		// Call function named MandatoryFlag
		Protection.MandatoryFlag(file, "f", "file")

		// Record the start time
		buildStartTime := time.Now()

		// Read source file version info
		sourceInfo, err := Manager.FindVersionInfo(file)
		if err != nil {
			fmt.Printf("Error reading source file: %v\n", err)
			os.Exit(1)
		}

		// Update target file with source file's version info
		err = UpdateVersionInfo(output, file, (*VersionInfo)(sourceInfo))
		if err != nil {
			fmt.Printf("Error updating target file: %v\n", err)
			os.Exit(1)
		}

		fmt.Println("Successfully updated version information")

		// Record the end time
		buildEndTime := time.Now()

		// Calculate the duration
		buildDurationTime := buildEndTime.Sub(buildStartTime)

		fmt.Printf("[+] PE Resources analysis completed in: %s\n\n", Colors.BoldYellow(buildDurationTime))

		return nil
	},
}

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

func ConvertVersionInfo(info *Manager.VersionInfo) *Output.VersionInfo {
	return &Output.VersionInfo{
		FileVersion:    info.FileVersion,
		ProductVersion: info.ProductVersion,
		FileFlagsMask:  info.FileFlagsMask,
		FileFlags:      info.FileFlags,
		FileOS:         info.FileOS,
		FileType:       info.FileType,
		FileSubtype:    info.FileSubtype,
		StringValues:   info.StringValues,
	}
}

// UpdateVersionInfo updates the version information in a PE file
func UpdateVersionInfo(sourceFile string, targetFile string, newInfo *VersionInfo) error {
	// Read the target file
	data, err := os.ReadFile(targetFile)
	if err != nil {
		return fmt.Errorf("error reading target file: %v", err)
	}

	// Create a copy to modify
	modifiedData := make([]byte, len(data))
	copy(modifiedData, data)

	// Open PE file for parsing
	r := bytes.NewReader(modifiedData)
	pefile, err := pe.NewFile(r)
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

	if virtualAddr == 0 || size == 0 {
		return fmt.Errorf("resource directory not found")
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
		return fmt.Errorf("version info section not found")
	}

	// Find VS_VERSION_INFO offset
	offset := findVersionInfoOffset(modifiedData)
	if offset == -1 {
		return fmt.Errorf("version info structure not found")
	}

	// Update fixed file info
	fixedInfoOffset := offset + 32 + 6 // Skip VS_VERSION_INFO header
	updateFixedFileInfo(modifiedData[fixedInfoOffset:], newInfo)

	// Update string file info
	stringInfoOffset := findStringFileInfoOffset(modifiedData[fixedInfoOffset:])
	if stringInfoOffset != -1 {
		updateStringFileInfo(modifiedData[fixedInfoOffset+stringInfoOffset:], newInfo)
	}

	// Write the modified data to a new file
	err = os.WriteFile(sourceFile, modifiedData, 0644)
	if err != nil {
		return fmt.Errorf("error writing modified file: %v", err)
	}

	return nil
}

func updateFixedFileInfo(data []byte, info *VersionInfo) {
	// Update File Version
	binary.LittleEndian.PutUint16(data[10:], info.FileVersion[0])
	binary.LittleEndian.PutUint16(data[8:], info.FileVersion[1])
	binary.LittleEndian.PutUint16(data[14:], info.FileVersion[2])
	binary.LittleEndian.PutUint16(data[12:], info.FileVersion[3])

	// Update Product Version
	binary.LittleEndian.PutUint16(data[18:], info.ProductVersion[0])
	binary.LittleEndian.PutUint16(data[16:], info.ProductVersion[1])
	binary.LittleEndian.PutUint16(data[22:], info.ProductVersion[2])
	binary.LittleEndian.PutUint16(data[20:], info.ProductVersion[3])

	// Update Flags and other fields
	binary.LittleEndian.PutUint32(data[24:], info.FileFlagsMask)
	binary.LittleEndian.PutUint32(data[28:], info.FileFlags)
	binary.LittleEndian.PutUint32(data[32:], info.FileOS)
	binary.LittleEndian.PutUint32(data[36:], info.FileType)
	binary.LittleEndian.PutUint32(data[40:], info.FileSubtype)
}

func updateStringFileInfo(data []byte, info *VersionInfo) {
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

	for _, key := range stringKeys {
		if newValue, exists := info.StringValues[key]; exists {
			replaceStringValue(data, key, newValue)
		}
	}
}

func replaceStringValue(data []byte, key string, newValue string) {
	pattern := make([]byte, len(key)*2)
	for i, c := range key {
		pattern[i*2] = byte(c)
		pattern[i*2+1] = 0x00
	}

	// Find key in data
	keyOffset := bytes.Index(data, pattern)
	if keyOffset == -1 {
		return
	}

	// Find value start (after key and padding)
	valueStart := keyOffset + len(pattern)
	for valueStart < len(data)-1 {
		if data[valueStart] == 0 && data[valueStart+1] == 0 {
			valueStart += 2
			break
		}
		valueStart++
	}

	// Write new value
	for i, c := range newValue {
		if valueStart+i*2+1 >= len(data) {
			break
		}
		data[valueStart+i*2] = byte(c)
		data[valueStart+i*2+1] = 0x00
	}
	// Null terminate
	if valueStart+len(newValue)*2+1 < len(data) {
		data[valueStart+len(newValue)*2] = 0x00
		data[valueStart+len(newValue)*2+1] = 0x00
	}
}

// Helper function to find VS_VERSION_INFO offset
func findVersionInfoOffset(data []byte) int {
	pattern := []byte{'V', 0x00, 'S', 0x00, '_', 0x00, 'V', 0x00, 'E', 0x00, 'R', 0x00, 'S', 0x00, 'I', 0x00, 'O', 0x00, 'N', 0x00, '_', 0x00, 'I', 0x00, 'N', 0x00, 'F', 0x00, 'O', 0x00}
	return bytes.Index(data, pattern)
}

// Helper function to find StringFileInfo offset
func findStringFileInfoOffset(data []byte) int {
	pattern := []byte{'S', 0x00, 't', 0x00, 'r', 0x00, 'i', 0x00, 'n', 0x00, 'g', 0x00, 'F', 0x00, 'i', 0x00, 'l', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 'f', 0x00, 'o', 0x00}
	return bytes.Index(data, pattern)
}
