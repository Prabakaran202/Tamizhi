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
|---|---|
| 🔥 **LLVM Backend** | Optimized native code generation for modern CPUs |
| ⚡ **C-Level Performance** | Significantly faster than Python and other interpreted languages |
| 🛠️ **Native CLI** | Dedicated `tamizhi` command-line tool written in C |
| 🌐 **Tamil Syntax** | Write logic using Tamil keywords (`அச்சிடு`, `முழுஎண்`, etc.) |
| 🐧 **Linux Native** | Optimized for Linux (Arch/Manjaro) and Android (Termux) |

---

## 🚀 Getting Started

##bash

# 1. முதல்ல உங்க लोकल ஃபோல்டர்ல இருக்குற ஸ்கிரிப்ட்டுக்கு எக்ஸிகியூட் பெர்மிஷன் கொடுங்க
chmod +x install.sh

# 2. இப்போ இந்த லோக்கல் இன்ஸ்டாலர் ஸ்கிரிப்ட்டை ரன் பண்ணுங்க
./install.sh


### Prerequisites

Ensure the following tools are installed on your system:

```bash
gcc --version
clang --version
llvm-config --version
```

### Installation

**1. Clone the repository:**

```bash
git clone https://github.com/Prabakaran202/Tamizhi.git
cd Tamizhi
```

**2. Build the compiler:**

```bash
make
```

**3. Install globally:**

```bash
sudo cp tamizhi tamizhi_core /usr/local/bin/
```

---

## 👨‍💻 Your First Tamizhi Program

Create a new file with `.tz` extension (e.g., `vanakkam.tz`):

```tamizhi
// Tamizhi V0.1 — Basic Arithmetic

Num அ = 100;
Num ஆ = 200;
Num இ = அ + ஆ;

அச்சிடு இ;
```

**Run it:**

```bash
tamizhi run vanakkam.tz
```

**Output:**

```
300
```

---

## 📁 Project Structure

```
Tamizhi/
├── src/                  # Lexer, Parser, and Code Generator (C source files)
├── include/              # Header files
├── examples/             # Sample Tamizhi programs
├── tamizhi               # User-facing CLI tool
├── tamizhi_core          # Compiler engine (LLVM-based)
└── Makefile
```

---

## 🗺️ Roadmap

- [x] LLVM Integration (Core Compiler Engine)
- [x] Native C CLI (`tamizhi run`)
- [x] Basic arithmetic and variable declaration
- [ ] Conditionals — `எனில்` / `இல்லையெனில்` (if / else)
- [ ] Loops — `மீண்டும்` (while / for)
- [ ] Functions — `செயல்` (functions)
- [ ] Python Library Bridge
- [ ] Tamizhi Package Manager (TPM)

---

## 🤝 Contributing

Tamizhi is an open-source project under the **Backend Developer Hub (BDH)** community. Contributions are welcome!

- 🐛 Found a bug? [Open an Issue](https://github.com/Prabakaran202/Tamizhi/issues)
- 💡 Have a feature idea? Submit a Pull Request
- ⭐ Like the project? Give it a star!

---
## 📊 Benchmarks

| 🔁 Loop Iterations | 1,000,000 |
| ⏱️ Execution Time | 0.24s (output suppressed) |
| ⚙️ CPU Usage | 64% |
| 📱 Device | Android (Termux) — aarch64 |
| 🔧 Backend | LLVM Native |
## 📜 License

This project is licensed under the [MIT License](LICENSE).

---

<div align="center">

Developed with ❤️ by [Prabakaran](https://github.com/Prabakaran202) · [Backend Developer Hub](https://github.com/BackendDeveloperHub)

*தமிழுக்கும் அமுதென்று பேர் — அந்தத் தமிழ் இன்பத் தமிழ் எங்கள் உயிருக்கு நேர்!*

</div>
