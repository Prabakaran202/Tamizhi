<div align="center">

# 🌟 Tamizhi (தமிழி)

**A Native Compiled Programming Language with Tamil Syntax**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Android-blue.svg)]()
[![Backend](https://img.shields.io/badge/Backend-LLVM-red.svg)]()
[![Community](https://img.shields.io/badge/Community-BackendDeveloperHub-green.svg)](https://github.com/BackendDeveloperHub)
[![Version](https://img.shields.io/badge/Version-0.1-orange.svg)]()

*Write code in Tamil. Compile to native machine code. Run at C speed.*

</div>

---

## 📖 About

**Tamizhi** is a compiled programming language that combines the elegance of the Tamil language with the raw performance of native machine code. Unlike interpreted languages, Tamizhi compiles directly to binary via an LLVM backend — delivering performance comparable to C, while letting developers write logic in Tamil syntax.

> தமிழிலேயே கோட் எழுதி, மெஷின் ஸ்பீட்-ல இயக்கு.

---

## ✨ Features

| Feature | Description |
| :--- | :--- |
| 🔥 **LLVM Backend** | Optimized native code generation for modern CPUs |
| ⚡ **C-Level Performance** | Significantly faster than Python and other interpreted languages |
| 🛠️ **Native CLI** | Dedicated `tamizhi` command-line tool written in C |
| 🌐 **Tamil Syntax** | Write logic using Tamil keywords (`அச்சிடு`, `Num`, etc.) |
| 🐧 **Linux Native** | Optimized for Linux (Arch/Manjaro) and Android (Termux) |

---

## 🚀 Getting Started

### Prerequisites

Ensure the following compiler tools are installed on your system:

- **Arch Linux / Manjaro:** `sudo pacman -S clang llvm make`
- **Ubuntu / Debian:** `sudo apt install clang llvm make -y`
- **Android Termux:** `pkg install clang llvm make`

### Installation (The Global Automated Way)

You can install the Tamizhi Compiler globally on your system with a single command. It will automatically download, compile, and configure the global environment.

```bash
curl -fsSL https://raw.githubusercontent.com/Prabakaran202/Tamizhi/main/install.sh | bash
```

Alternatively, for local development:

```bash
git clone https://github.com/Prabakaran202/Tamizhi.git
cd Tamizhi
chmod +x install.sh
./install.sh
```

---

## 👨‍💻 Your First Tamizhi Program

Create a new file with `.tz` extension (e.g., `vanakkam.tz`):

```tamizhi
fun main {
    Num அ = 100 ;
    Num ஆ = 200 ;
    Num இ = அ + ஆ ;

    print "கூட்டல் விடை:" ;
    print இ ;
}
```

Run it anywhere on your system:

```bash
tamizhi run vanakkam.tz
```

**Output:**

```
கூட்டல் விடை:
300
```

---

## 📁 Project Structure

```
Tamizhi/
├── src/                  # Lexer, Parser, and LLVM Code Generator (C source files)
├── include/              # Compiler Core Header files
├── examples/             # Sample Tamizhi advanced programs (.tz)
├── .vscode/              # Pre-configured global tasks for VS Code execution
├── install.sh            # Global one-click automated installer script
└── Makefile              # Automated Clang compilation pipelines
```
```
| Tamil          | English   | Token Type | Category   |
|----------------|-----------|------------|------------|
| முதன்மை        | main      | T_MAIN     | Structure  |
| நிகழ்          | fun       | T_FUNC     | Structure  |
| பூட்டர்        | footer    | T_FOOTER   | Structure  |
| அச்சிடு        | print     | T_PRINT    | I/O        |
| உள்ளீடு        | input     | T_INP      | I/O        |
| எண் / முழுஎண்  | Num       | T_INT      | Data Type  |
| வரி            | Str       | T_STR      | Data Type  |
| உண்மை          | bool      | T_BOOL     | Data Type  |
| எனில்          | if        | T_IF       | Control    |
| இல்லையெனில்    | else      | T_ELSE     | Control    |
| சு             | for       | T_FOR      | Control    |
| சு2            | while     | T_WHILE    | Control    |
| திரும்பு       | return    | T_RET      | Other      |
| இயக்கு         | call      | T_CALL     | Other      |
| வரிசை          | line      | T_LINE     | Other      |
```
---

## 🗺️ Roadmap

- [x] LLVM Integration (Core Compiler Engine)
- [x] Native C CLI (`tamizhi run`)
- [x] Global Universal Installer Script (`install.sh`)
- [x] Precedence Engine Matrix (AST Tree Walker)
- [x] Conditions — `if` / `else` Block Parsing
- [x] Loops — `for` Iteration Engines
- [ ] Conditionals — `எனின்தான்` / `இல்லையெனில்` (Tamil Syntax Mapping)
- [ ] Return Infrastructure (return values from functions)
- [ ] Floating Point Support (Float / Decimal operations)
- [ ] Tamizhi Package Manager (TPM)

---

## 🤝 Contributing

Tamizhi is an open-source project under the **Backend Developer Hub (BDH)** community. Contributions are highly welcome!

- 🐛 Found a bug or syntax error? [Open an Issue](https://github.com/Prabakaran202/Tamizhi/issues)
- 💡 Have a cool core feature idea? Submit a Pull Request
- ⭐ Like the concept? Give this repository a star!

---

## 📊 Benchmarks

| Metric | Result |
| :--- | :--- |
| 🔁 Loop Iterations | 1,000,000 |
| ⏱️ Execution Time | 0.24s (AOT Native Compilation) |
| ⚙️ CPU Usage | 64% |
| 📱 Device | Android (Termux) — aarch64 / Linux x86_64 |
| 🔧 Backend | LLVM Native Toolchain |

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).

---

<div align="center">

Developed with ❤️ by [Prabakaran](https://github.com/Prabakaran202) · [Backend Developer Hub](https://github.com/BackendDeveloperHub)

*தமிழுக்கும் அமுதென்று பேர் — அந்தத் தமிழ் இன்பத் தமிழ் எங்கள் உயிருக்கு நேர்!*

</div>
