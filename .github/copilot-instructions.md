# Copilot Instructions — Multi-Component Hardware Project

## Project Identity

- **What**: A unified repository containing firmware, a host listener, a web configurator, and documentation for a custom hardware product.
- **Structure**: This is a monorepo. Rules are strictly scoped to specific directories.
- **Exclusions**: NEVER provide suggestions, code, or context for the `hardware/` directory. It contains KiCad/FreeCAD files only.

---

## 1. Firmware (`/firmware/`)

**Environment**: C++, PlatformIO, ESP32/AVR (Embedded)

- **Detailed Workflow**: ALWAYS reference `firmware/platformio-workflow.md` for extended guidelines and procedural steps.
- **Architecture**: Strict separation between hardware initialization and application logic.
- **Dependencies**: ALWAYS use `platformio.ini` `lib_deps` with strict semantic versioning. NEVER assume global Arduino IDE libraries.
- **Hardware Config**: NEVER hardcode pins or magic numbers in `.cpp` files. ALWAYS read from `include/config.h` using `constexpr` variables.
- **Coding Rules**:
    - ALWAYS use `constexpr` over `#define` for constants.
    - ALWAYS use fixed-width integers (`uint8_t`, `uint32_t`, `int16_t`). NEVER use `int` or `long`.
    - NEVER use blocking code like `delay()`. ALWAYS use non-blocking `millis()` timing logic.
    - Avoid dynamic memory allocation (`new`, `malloc`, `String`). Prefer standard C-strings or static allocation.
- **Naming Conventions**: `PascalCase` for classes, `snake_case` for functions, `UPPER_SNAKE_CASE` for constants, `_camelCase` for private class methods.

---

## 2. Host Listener (`/host-listener/`)

**Environment**: Python 3.10+

- **Detailed Workflow**: ALWAYS reference `host-listener/python-workflow.md` for extended guidelines and procedural steps.
- **Architecture**: Modular Python application.
- **Dependencies**: ALWAYS assume dependencies are managed via a virtual environment and listed in `requirements.txt`.
- **Coding Rules**:
    - ALWAYS use explicit type hints for function arguments and return types.
    - ALWAYS adhere to PEP 8 standards.
    - NEVER hardcode secrets or API keys. ALWAYS use environment variables.
    - ALWAYS include `if __name__ == "__main__":` guards in executable scripts.
- **Naming Conventions**: `PascalCase` for classes, `lower_snake_case` for variables/functions/modules, `UPPER_SNAKE_CASE` for constants. Use a leading underscore `_` for internal/private methods.

---

## 3. Web Configurator (`/web-configurator/`)

**Environment**: Vanilla HTML, CSS, JavaScript (No frameworks)

- **Detailed Workflow**: ALWAYS reference `web-configurator/web-design-workflow.md` for extended guidelines and procedural steps.
- **Architecture**: Modular vanilla web design enforcing the Single Responsibility Principle (SRP).
- **Dependencies**: NEVER suggest npm packages, Node modules, React, Vue, or Tailwind. Rely ONLY on vanilla code or standard CDNs (e.g., FontAwesome, Google Fonts).
- **Coding Rules**:
    - **HTML**: ALWAYS use semantic tags (`<header>`, `<main>`, `<section>`). NEVER use inline styles.
    - **CSS**: ALWAYS use native CSS variables (`:root`). ALWAYS use the BEM methodology for classes (e.g., `card__title--active`). NEVER use IDs for styling.
    - **JS**: ALWAYS use ES6+ (`const`/`let`, arrow functions). ALWAYS use ES Modules (`import`/`export`).
    - **JS Separation**: Keep DOM manipulation, API/Network calls, and pure utility functions in separate files.
- **Naming Conventions**: Prefix variables storing DOM elements with `$` (e.g., `const $submitButton`). `camelCase` for JS functions, `kebab-case` for HTML/CSS files.

---

## 4. Documentation (`/docs/`)

**Environment**: Markdown

- **Languages**: The documentation exists in both Romanian (`README_RO.md`, `Documentatie.md`) and English (`Documentation.md`).
- **Rule**: ALWAYS reply or generate text in the language of the file you are currently editing.
- **Formatting**:
    - ALWAYS use standard Markdown formatting (ATX headings `#`, tables, bolding).
    - NEVER use raw HTML in the markdown files unless absolutely necessary for image alignment.
    - When referencing file paths or code variables in text, ALWAYS wrap them in backticks (e.g., `src/main.cpp`).
