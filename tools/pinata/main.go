/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#############################################################################
*/

package main

import (
	"archive/zip"
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"
)

const pinataUploadURL = "https://uploads.pinata.cloud/v3/files"

// progressReader wraps an io.Reader and prints a live progress bar to stderr.
type progressReader struct {
	r     io.Reader
	total int64
	read  int64
	start time.Time
	last  time.Time
}

func (p *progressReader) Read(buf []byte) (int, error) {
	n, err := p.r.Read(buf)
	p.read += int64(n)
	now := time.Now()
	if now.Sub(p.last) > 80*time.Millisecond || err == io.EOF {
		p.last = now
		elapsed := now.Sub(p.start).Seconds()
		if elapsed < 0.001 {
			elapsed = 0.001
		}
		speed := float64(p.read) / elapsed
		pct := float64(p.read) / float64(p.total) * 100.0
		bar := buildBar(pct, 28)
		fmt.Fprintf(os.Stderr, "\r  [%s] %5.1f%%  %s / %s  @ %s/s   ",
			bar, pct, fmtSize(p.read), fmtSize(p.total), fmtSize(int64(speed)))
	}
	return n, err
}

func buildBar(pct float64, width int) string {
	filled := int(pct / 100.0 * float64(width))
	if filled > width {
		filled = width // clamp — pct can slightly exceed 100 during last read
	}
	return strings.Repeat("█", filled) + strings.Repeat("░", width-filled)
}

func fmtSize(b int64) string {
	const (
		KB = 1024
		MB = 1024 * KB
		GB = 1024 * MB
	)
	switch {
	case b >= GB:
		return fmt.Sprintf("%.2f GB", float64(b)/float64(GB))
	case b >= MB:
		return fmt.Sprintf("%.2f MB", float64(b)/float64(MB))
	case b >= KB:
		return fmt.Sprintf("%.1f KB", float64(b)/float64(KB))
	default:
		return fmt.Sprintf("%d B", b)
	}
}

// zipDir zips srcDir into destZip, preserving relative paths inside the dir.
func zipDir(srcDir, destZip string) error {
	zf, err := os.Create(destZip)
	if err != nil {
		return err
	}
	defer zf.Close()
	zw := zip.NewWriter(zf)
	defer zw.Close()

	base := filepath.Dir(filepath.Clean(srcDir))
	return filepath.Walk(srcDir, func(path string, info os.FileInfo, err error) error {
		if err != nil || info.IsDir() {
			return err
		}
		rel, err := filepath.Rel(base, path)
		if err != nil {
			return err
		}
		w, err := zw.Create(filepath.ToSlash(rel))
		if err != nil {
			return err
		}
		f, err := os.Open(path)
		if err != nil {
			return err
		}
		defer f.Close()
		_, err = io.Copy(w, f)
		return err
	})
}

type pinataResponse struct {
	Data struct {
		CID  string `json:"cid"`
		Name string `json:"name"`
		Size int64  `json:"size"`
	} `json:"data"`
}

func pinFile(jwt, filePath, name string, public bool) error {
	f, err := os.Open(filePath)
	if err != nil {
		return fmt.Errorf("open: %w", err)
	}
	defer f.Close()

	fi, err := f.Stat()
	if err != nil {
		return fmt.Errorf("stat: %w", err)
	}

	fmt.Printf("  File : %s\n", name)
	fmt.Printf("  Size : %s\n\n", fmtSize(fi.Size()))

	// Build multipart body in a buffer so we know Content-Length upfront.
	// For files up to a few hundred MB this is fine; memory is cheaper than
	// chunked transfer which some CDN endpoints handle poorly.
	var body bytes.Buffer
	mw := multipart.NewWriter(&body)

	fw, err := mw.CreateFormFile("file", name)
	if err != nil {
		return fmt.Errorf("create form file: %w", err)
	}
	if _, err = io.Copy(fw, f); err != nil {
		return fmt.Errorf("buffer file: %w", err)
	}
	if err = mw.WriteField("name", name); err != nil {
		return fmt.Errorf("write name: %w", err)
	}
	network := "private"
	if public {
		network = "public"
	}
	if err = mw.WriteField("network", network); err != nil {
		return fmt.Errorf("write network: %w", err)
	}
	mw.Close()

	total := int64(body.Len())
	pr := &progressReader{
		r:     &body,
		total: total,
		start: time.Now(),
		last:  time.Now(),
	}

	req, err := http.NewRequest(http.MethodPost, pinataUploadURL, pr)
	if err != nil {
		return fmt.Errorf("build request: %w", err)
	}
	req.ContentLength = total
	req.Header.Set("Authorization", "Bearer "+jwt)
	req.Header.Set("Content-Type", mw.FormDataContentType())

	resp, err := (&http.Client{}).Do(req)
	fmt.Fprintln(os.Stderr) // newline after progress bar
	if err != nil {
		return fmt.Errorf("request failed: %w", err)
	}
	defer resp.Body.Close()

	respBody, _ := io.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK && resp.StatusCode != http.StatusCreated {
		return fmt.Errorf("Pinata API returned %d:\n%s", resp.StatusCode, string(respBody))
	}

	var result pinataResponse
	if err = json.Unmarshal(respBody, &result); err != nil {
		return fmt.Errorf("parse response: %w\n%s", err, string(respBody))
	}
	if result.Data.CID == "" {
		return fmt.Errorf("no CID in response:\n%s", string(respBody))
	}

	fmt.Printf("Pinned successfully.\n")
	fmt.Printf("  CID  : %s\n", result.Data.CID)
	fmt.Printf("  URI  : ipfs://%s\n", result.Data.CID)
	fmt.Printf("  HTTP : https://ipfs.io/ipfs/%s\n", result.Data.CID)
	fmt.Printf("  HTTP : https://dweb.link/ipfs/%s\n", result.Data.CID)
	return nil
}

func printUsage() {
	fmt.Println("Usage:")
	fmt.Println("  forgecli-pinata pin <file-or-dir> [--name <name>] [--private]")
	fmt.Println()
	fmt.Println("Environment:")
	fmt.Println("  PINATA_JWT   Pinata API JWT (required, never passed as CLI arg)")
	fmt.Println()
	fmt.Println("Examples:")
	fmt.Println("  PINATA_JWT=eyJ... forgecli-pinata pin release.apk")
	fmt.Println("  PINATA_JWT=eyJ... forgecli-pinata pin assets/ --name bundle.zip")
}

func main() {
	args := os.Args[1:]
	if len(args) == 0 || args[0] == "help" || args[0] == "--help" || args[0] == "-h" {
		printUsage()
		return
	}
	if args[0] != "pin" {
		fmt.Fprintf(os.Stderr, "Unknown command: %s\n\n", args[0])
		printUsage()
		os.Exit(1)
	}

	var inputPath, name string
	isPublic := true // default: public so gateway URLs work without auth
	for i := 1; i < len(args); i++ {
		switch args[i] {
		case "--name":
			i++
			if i >= len(args) {
				fmt.Fprintln(os.Stderr, "Error: --name requires a value")
				os.Exit(1)
			}
			name = args[i]
		case "--private":
			isPublic = false
		default:
			if strings.HasPrefix(args[i], "--") {
				fmt.Fprintf(os.Stderr, "Unknown option: %s\n", args[i])
				os.Exit(1)
			}
			inputPath = args[i]
		}
	}

	if inputPath == "" {
		fmt.Fprintln(os.Stderr, "Error: no file or directory specified.")
		fmt.Fprintln(os.Stderr)
		printUsage()
		os.Exit(1)
	}

	jwt := os.Getenv("PINATA_JWT")
	if jwt == "" {
		fmt.Fprintln(os.Stderr, "Error: PINATA_JWT environment variable is not set.")
		os.Exit(1)
	}

	fi, err := os.Stat(inputPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	uploadPath := inputPath
	isTemp := false

	if fi.IsDir() {
		base := filepath.Base(filepath.Clean(inputPath))
		tmpZip := filepath.Join(os.TempDir(), "forgecli_pinata_"+base+".zip")
		fmt.Printf("Zipping '%s'...\n", base)
		if err := zipDir(inputPath, tmpZip); err != nil {
			fmt.Fprintf(os.Stderr, "Error zipping: %v\n", err)
			os.Exit(1)
		}
		uploadPath = tmpZip
		isTemp = true
		if name == "" {
			name = base + ".zip"
		}
	}

	if name == "" {
		name = filepath.Base(uploadPath)
	}

	if isTemp {
		defer os.Remove(uploadPath)
	}

	fmt.Println("Uploading to Pinata...")
	if err := pinFile(jwt, uploadPath, name, isPublic); err != nil {
		fmt.Fprintf(os.Stderr, "\nError: %v\n", err)
		os.Exit(1)
	}
}
