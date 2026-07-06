<div align="center">

# 🌟 Tamizhi (தமிழி)

### *A Native Compiled Programming Language with Tamil Syntax*

[![License - GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform - Linux | Android](https://img.shields.io/badge/Platform-Linux%20%7C%20Android-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Backend - LLVM](https://img.shields.io/badge/Backend-LLVM-red.svg)](https://llvm.org/)
[![Community - BackendDeveloperHub](https://img.shields.io/badge/Community-BackendDeveloperHub-green.svg)](https://github.com/BackendDeveloperHub)
[![PyPI - Tamizhi](https://img.shields.io/badge/PyPI-Tamizhi-blue.svg)](https://pypi.org/project/Tamizhi/)
[![Version - 0.1](https://img.shields.io/badge/Version-0.1-orange.svg)](https://pypi.org/project/Tamizhi/)

**Write code in Tamil. Compile to native machine code. Run at C speed.**

</div>

---

## 📖 About

Tamizhi is a compiled programming language that combines the elegance of the Tamil language with the raw performance of native machine code. Unlike interpreted languages, Tamizhi compiles directly to binary via an LLVM backend—delivering performance comparable to C, while letting developers write logic in Tamil syntax.

> «தமிழிலேயே கோட் எழுதி, மெஷின் ஸ்பீட்-ல இயக்கு.»

---

## ✨ Features

| Feature | Description |
| :--- | :--- |
| **🔥 LLVM Backend** | Optimized native code generation for modern CPUs. |
| **⚡ C-Level Performance** | Significantly faster than Python and other interpreted languages. |
| **🛠️ Native CLI** | Dedicated `tamizhi` command-line tool written in C. |
| **🌐 Tamil Syntax** | Write logic using Tamil keywords (`அச்சிடு`, `Num`, etc.). |
| **🐧 Linux Native** | Optimized for Linux (Arch/Manjaro) and Android (Termux). |

---

## 🚀 Getting Started

### Prerequisites

Ensure the following compiler tools are installed on your system before installation:

*   **Arch Linux / Manjaro:** `sudo pacman -S clang llvm make`
*   **Ubuntu / Debian:** `sudo apt install clang llvm make -y`
*   **Android Termux:** `pkg install clang llvm make`

---

## 🛠️ Installation & Setup (Local Development)
For development, testing, or contributing to Tamizhi, clone the repository and run the automated local installer:

```bash
# Clone the repository
git clone [https://github.com/Prabakaran202/Tamizhi.git](https://github.com/Prabakaran202/Tamizhi.git)

# Navigate into the project directory
cd Tamizhi

# Give execution permission to the installer
chmod +x install.sh

# Run the automated local installer
./install.sh
```
```bash
###pip
pipnstall Tamizhi
cd tamizhi-extract
./install.sh
```
<h2>📁 Project Structure</h2>
<pre>

Tamizhi
├── core
│   ├── decoder.c
│   ├── encoder.c
│   └── http_runtime.c
├── examples
│   ├── loop.tz
│   ├── Main.tz
│   ├── tamizhiv01.tz
│   └── V.tz
├── include
│   ├── ast.h
│   ├── codegen_bridge.h
│   ├── codegen.h
│   ├── lexer.h
│   └── parser.h
├── index.html
├── install.sh
├── lib
│   ├── http.tz
│   └── thirai.tz
├── LICENSE
├── log.HTML 
├── Makefile
├── README.md
├── src
│   ├── ast.c
│   ├── codegen
│   │   ├── cg_alloca.c
│   │   ├── cg_assign_ret.c
│   │   ├── cg_bitcode.c
│   │   ├── cg_dna.c
│   │   ├── cg_else_start.c
│   │   ├── cg_entry.c
│   │   ├── cg_eval_ast.c
│   │   ├── cg_finish_destroy.c
│   │   ├── cg_func_call.c
│   │   ├── cg_func_end.c
│   │   ├── cg_func_start.c
│   │   ├── cg_http.c
│   │   ├── cg_if_body_end.c
│   │   ├── cg_if_end.c
│   │   ├── cg_if_start.c
│   │   ├── cg_init.c
│   │   ├── cg_loop_end.c
│   │   ├── cg_loop_start.c
│   │   ├── cg_math_ast.c
│   │   ├── cg_math_op.c
│   │   ├── cg_print.c
│   │   ├── cg_return.c
│   │   ├── cg_str.c
│   │   ├── cg_system_call.c
│   │   ├── cg_ternary.c
│   │   ├── cg_trim.c
│   │   └── cg_var.c
│   ├── codegen.c
│   ├── lexer.c
│   ├── main.c
│   ├── parser.c
│   └── tamizhi_cli.c
├── tamizhi
├── tamizhi-py
│   ├── pyproject.toml
│   ├── README.md
│   ├── setup.py
│   └── tamizhi
│       ├── cli.py
│       └── __init__.py
</pre>
<h2>🗺️ Roadmap</h2>

    [x] LLVM Integration (Core Compiler Engine)

    [x] Native C CLI (tamizhi run)

    [x] Global Installer (install.sh)

    [x] AST Tree Walker & Precedence Engine

    [x] if / else Parsing

    [x] for Loop Engine

    [x] PyPI Distribution (pip install Tamizhi)

    [ ] Tamil Conditional Keywords (எனின்தான், இல்லையெனில்)

    [ ] Function Return Values

    [ ] Float & Decimal Support

    [ ] Tamizhi Package Manager (TPM)


<h1>🤝 Contributing</h1>

<h2>Tamizhi is an open-source project under the BackendDeveloperHub community. Contributions are welcome from everyone!</h2>
    
    🐛 Found a bug? Open an issue.

    💡 Have an idea? Submit a pull request.

    ⭐ Support the project by starring the repository!


<h1>📦 Links</h1>

    🐙 GitHub: https://github.com/Prabakaran202/Tamizhi

    🏢 BackendDeveloperHub: https://github.com/BackendDeveloperHub

    🐍 PyPI: https://pypi.org/project/Tamizhi/

<h1>📜 License</h1>

<h2>Tamizhi is licensed under the GNU General Public License v3.0 (GPLv3).</h2>

<h3>This means:</h3>

    ✅ You can use, modify, and distribute the project.

    ✅ Contributions remain open-source.

    ⚠️ Any modified distributed version must also remain under GPLv3.

Read the full license here: GNU GPLv3

<div  align="center" textcolor = "green"> Developed with ❤️ by Prabakaran · BackendDeveloperHub</div>






























