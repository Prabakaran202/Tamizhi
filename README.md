<div align="center">🌟 Tamizhi (தமிழி)

A Native Compiled Programming Language with Tamil Syntax

""License: GPLv3" (https://img.shields.io/badge/License-GPLv3-blue.svg)" (https://www.gnu.org/licenses/gpl-3.0)
""Platform" (https://img.shields.io/badge/Platform-Linux%20%7C%20Android-blue.svg)"
""Backend" (https://img.shields.io/badge/Backend-LLVM-red.svg)"
""Community" (https://img.shields.io/badge/Community-BackendDeveloperHub-green.svg)" (https://github.com/BackendDeveloperHub)
""PyPI" (https://img.shields.io/badge/PyPI-Tamizhi-blue.svg)" (https://pypi.org/project/Tamizhi/)
""Version" (https://img.shields.io/badge/Version-0.1-orange.svg)"

Write code in Tamil. Compile to native machine code. Run at C speed.

</div>---

📖 About

Tamizhi is a compiled programming language that combines the elegance of the Tamil language with the raw performance of native machine code. Unlike interpreted languages, Tamizhi compiles directly to binary via an LLVM backend — delivering performance comparable to C, while letting developers write logic in Tamil syntax.

«தமிழிலேயே கோட் எழுதி, மெஷின் ஸ்பீட்-ல இயக்கு.»

---

✨ Features

Feature| Description
🔥 LLVM Backend| Optimized native code generation for modern CPUs
⚡ C-Level Performance| Significantly faster than Python and other interpreted languages
🛠️ Native CLI| Dedicated "tamizhi" command-line tool written in C
🌐 Tamil Syntax| Write logic using Tamil keywords ("அச்சிடு", "Num", etc.)
🐧 Linux Native| Optimized for Linux (Arch/Manjaro) and Android (Termux)

---

🚀 Getting Started

Prerequisites

Ensure the following compiler tools are installed on your system:

- Arch Linux / Manjaro: "sudo pacman -S clang llvm make"
- Ubuntu / Debian: "sudo apt install clang llvm make -y"
- Android Termux: "pkg install clang llvm make"

Installation

Option 1: Install via PyPI (Recommended)

Tamizhi is published on PyPI: "pypi.org/project/Tamizhi" (https://pypi.org/project/Tamizhi/)

pip install Tamizhi
cd tamizhi-extract
./install.sh

Option 2: Global Automated Installation

Install Tamizhi globally with one command:

curl -fsSL https://raw.githubusercontent.com/Prabakaran202/Tamizhi/main/install.sh | bash

Option 3: Local Development

git clone https://github.com/Prabakaran202/Tamizhi.git
cd Tamizhi
chmod +x install.sh
./install.sh

---

👨‍💻 Your First Tamizhi Program

Create a new file with ".tz" extension:

fun main {
    Num அ = 100 ;
    Num ஆ = 200 ;
    Num இ = அ + ஆ ;

    print "கூட்டல் விடை:" ;
    print இ ;
}

Run the program:

tamizhi run vanakkam.tz

Output

கூட்டல் விடை:
300

---

📁 Project Structure

Tamizhi/
├── src/                  # Lexer, Parser, LLVM Code Generator
├── include/              # Compiler Core Headers
├── examples/             # Example Tamizhi Programs
├── .vscode/              # VS Code Tasks
├── install.sh            # Global Installer
└── Makefile              # Build Automation

---

🗺️ Roadmap

- [x] LLVM Integration (Core Compiler Engine)
- [x] Native C CLI ("tamizhi run")
- [x] Global Installer ("install.sh")
- [x] AST Tree Walker & Precedence Engine
- [x] "if" / "else" Parsing
- [x] "for" Loop Engine
- [x] PyPI Distribution ("pip install Tamizhi")
- [ ] Tamil Conditional Keywords ("எனின்தான்", "இல்லையெனில்")
- [ ] Function Return Values
- [ ] Float & Decimal Support
- [ ] Tamizhi Package Manager (TPM)

---

🤝 Contributing

Tamizhi is an open-source project under the BackendDeveloperHub community. Contributions are welcome from everyone.

- 🐛 Found a bug? Open an issue
- 💡 Have an idea? Submit a pull request
- ⭐ Support the project by starring the repository

---

📊 Benchmarks

Metric| Result
🔁 Loop Iterations| 1,000,000
⏱️ Execution Time| 0.24s
⚙️ Compilation Type| AOT Native Compilation
📱 Device| Android (Termux) / Linux x86_64
🔧 Backend| LLVM Native Toolchain

---

📦 Links

- 🐍 PyPI: https://pypi.org/project/Tamizhi/
- 🐙 GitHub: https://github.com/Prabakaran202/Tamizhi
- 🏢 BackendDeveloperHub: https://github.com/BackendDeveloperHub

---

📜 License

Tamizhi is licensed under the GNU General Public License v3.0 (GPLv3).

This means:

- ✅ You can use, modify, and distribute the project
- ✅ Contributions remain open-source
- ⚠️ Any modified distributed version must also remain under GPLv3

Read the full license here:
https://www.gnu.org/licenses/gpl-3.0.en.html

---

<div align="center">Developed with ❤️ by "Prabakaran" (https://github.com/Prabakaran202) · "BackendDeveloperHub" (https://github.com/BackendDeveloperHub)

தமிழுக்கும் அமுதென்று பேர் — அந்தத் தமிழ் இன்பத் தமிழ் எங்கள் உயிருக்கு நேர்!

</div>
