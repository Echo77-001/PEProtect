# PEProtect

A PE file packer for Windows x64.

> [!IMPORTANT]
> **Educational Purposes Only:** This project is created strictly for educational and research purposes to understand the inner workings of the PE file format, loaders, and memory management in Windows. It is not intended for malicious use or deployment in production environments.

## Features

* Reads the original EXE, parses sections, the import table, and relocations.
* Manually restores everything at runtime.
* Manually resolves function addresses using the PEB.

## Analysis & Tests

## Screenshots

<p align="center">
  <b>Before (Original File)</b><br>
  <img src="images/before.png" alt="Before Packing" width="100%">
</p>

<br>

<p align="center">
  <b>After (Packed with PEProtect)</b><br>
  <img src="images/after.png" alt="After Packing" width="100%">
</p>

<p align="center">
  <em>Example of a file before and after packing with PEProtect</em>
</p>

You can check out the VirusTotal test results for a sample file processed with this tool:
* [VirusTotal Report (Before Packing)](https://www.virustotal.com/gui/file/127cd97708944110d0f13bbc7b1e912cc6cf12740b16e1c15fd5562e687a83f1/detection)
* [VirusTotal Report (After Packing)](https://www.virustotal.com/gui/file/627d334cb41ab3cb3045a0f7271c3a9bd56502c468530f20842bd395966a6705?nocache=1)

## Building

### Prerequisites
* Visual Studio 2022+
* ZSTD
* CMake

### Commands
```cmd
cmake -S . -B build
cmake --build build --config Release
```

## Usage

```text
Usage: PEProtect input.exe [options]

Options:
  -printData    Print data dump for debugging
  -noreloc      Remove relocation table
  -showSection  Prints info about all sections

Example:
  PEProtect target.exe -printData
  PEProtect calc.exe
```

## To-Do

- [ ] **Mutation Engine** Junk code insertion & instruction substitution.
- [ ] **Anti-Debugging** `IsDebuggerPresent`, PEB checks & `RDTSC` timing.
- [ ] **Code Virtualization** Custom bytecode interpreter for sensitive functions.