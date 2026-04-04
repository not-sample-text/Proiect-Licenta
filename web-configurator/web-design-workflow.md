# Web Design Project Guide

This guide covers the recommended folder layout, file management, and coding guidelines for vanilla web design projects (HTML, CSS, JS).

The primary goals of this architecture are **modularity** and strict adherence to the **Single Responsibility Principle (SRP)**. Even without a framework, web projects MUST remain maintainable, scalable, and easy to navigate.

## Folder Layout

Every vanilla web project follows this minimal structure:

```text
<project-name>/
├── .github/
│   └── copilot-instructions.md        # Copilot instructions file
├── .vscode/
│   └── settings.json                  # VSCode workspace settings
├── docs/
│   └── 01_specificatii-initiale.md    # Initial project specifications (from client)
├── public/                            # Web root / deployment directory
│   ├── assets/                        # Static media (images, fonts, icons)
│   ├── css/                           # Stylesheets
│   ├── js/                            # JavaScript modules
│   └── index.html                     # Main entry point
├── .gitattributes                     # Git attributes file for language-specific settings
├── .gitignore                         # Git ignore file
└── README.md                          # Project overview and documentation
```

### An Example with Modularity and SRP

As the project grows, files MUST be split logically. Do not dump all styles into a single `style.css` or all logic into `script.js`.

```text
<project-name>/
├── docs/
│   ├── 01_specificatii-initiale.md
│   └── 02_design-mockups/
├── public/
│   ├── assets/
│   │   ├── fonts/
│   │   ├── icons/
│   │   └── images/
│   ├── css/
│   │   ├── base/                      # Resets, typography, and CSS variables
│   │   │   ├── reset.css
│   │   │   └── variables.css
│   │   ├── layout/                    # Grid/flexbox layouts, header, footer
│   │   │   ├── grid.css
│   │   │   └── header.css
│   │   ├── components/                # Modular, standalone UI elements
│   │   │   ├── buttons.css
│   │   │   ├── cards.css
│   │   │   └── modals.css
│   │   └── main.css                   # Entry CSS file (imports the others)
│   ├── js/
│   │   ├── api/                       # Network requests and data fetching
│   │   │   └── httpClient.js
│   │   ├── components/                # UI logic, scoped to specific elements
│   │   │   ├── modal.js
│   │   │   └── formValidator.js
│   │   ├── utils/                     # Pure functions, helpers (no DOM logic)
│   │   │   └── dateFormatter.js
│   │   └── app.js                     # Main JS entry point (initializes components)
│   ├── index.html                     # Homepage
│   └── about.html                     # Secondary page
└── README.md
```

## Naming Conventions

### Files & Folders

- **HTML files**: `kebab-case.html` (e.g., `contact-us.html`)
- **CSS files**: `kebab-case.css` (e.g., `product-card.css`)
- **JS files**: `camelCase.js` or `kebab-case.js` (pick one and be consistent, e.g., `formValidator.js`)
- **Assets/Images**: `kebab-case.extension` (e.g., `hero-background.jpg`, `icon-menu.svg`)

### Code Identifiers

- **CSS Classes**: Use **BEM (Block Element Modifier)** methodology for components. `block__element--modifier` (e.g., `card`, `card__title`, `card--highlighted`). NEVER use IDs for styling.
- **JS Variables & Functions**: `camelCase` (e.g., `fetchData()`, `userList`)
- **JS Classes**: `PascalCase` (e.g., `ModalController`, `DataFetcher`)
- **JS Constants**: `UPPER_SNAKE_CASE` (e.g., `API_BASE_URL`, `MAX_RETRIES`)
- **JS DOM Elements**: Prefix variables holding DOM elements with `$` to distinguish them from standard data variables (e.g., `const $submitButton = document.querySelector('.btn-submit');`).

## Coding Guidelines

### HTML: Semantic & Accessible

- ALWAYS use semantic tags (`<header>`, `<main>`, `<article>`, `<section>`, `<nav>`, `<footer>`). NEVER rely purely on `<div>` and `<span>`.
- ALWAYS include `alt` attributes on images.
- NEVER use inline styles (e.g., `<div style="color: red;">`).

### CSS: Modular & Scalable

- **CSS Variables**: ALWAYS use native CSS variables (`:root`) for colors, fonts, and spacing. This ensures consistency and makes theme switching trivial.
- **Mobile-First**: ALWAYS write base styles for mobile devices first, then use `min-width` media queries to add layout complexity for larger screens.
- **Avoid `!important`**: NEVER use `!important` unless absolutely necessary (e.g., utility classes). Fix your specificity instead.
- **Separation of Concerns**: Keep layout rules (grid/flex) separated from component aesthetic rules (colors, padding, borders).

### JavaScript: SRP & Modern Standards

- **Use ES6+**: Use `const` and `let`. NEVER use `var`. Use arrow functions, template literals, and destructuring.
- **ES Modules**: Use `<script type="module" src="js/app.js"></script>`. Split your code using `import` and `export`.
- **Single Responsibility Principle**: A JavaScript file/class MUST do one thing.
    - **Data Fetching** belongs in `api/` (returns JSON/Promises).
    - **DOM Manipulation** belongs in `components/` (updates the UI).
    - NEVER mix HTML generation, API calls, and event listeners in a single massive function.
- **Event Delegation**: Attach event listeners to parent containers rather than individual child elements, especially for dynamically generated content.

## Dependencies (CDNs)

Since this is a vanilla project, avoid `npm` or `node_modules` for frontend libraries unless you are introducing a build step (like Vite/Webpack). Rely on CDNs.

If you use external libraries, document them in your `README.md`. **NEVER** introduce a library (like jQuery or lodash) for something that can easily be written in modern vanilla JS.

## README.md Structure

Every web project **MUST** include a `README.md` file in the repository root.

```md
# <Project Name>

One or two sentences describing what the web project does and its purpose.

## Environment

- Target Browsers: Modern browsers (Chrome, Firefox, Safari, Edge)
- Deployment: [Static hosting platform, e.g., GitHub Pages, Netlify, Apache]

## External Dependencies

List all external libraries, fonts, or frameworks loaded via CDN:

| Resource     | Version | Purpose                    |
| ------------ | ------- | -------------------------- |
| Google Fonts | N/A     | Inter and Roboto fonts     |
| FontAwesome  | 6.4.0   | UI Icons                   |
| Chart.js     | 4.3.0   | Rendering dashboard charts |

## Architecture Notes

- **CSS Methodology**: BEM
- **JS Architecture**: ES Modules (Vanilla JS). Entry point is `public/js/app.js`.

## Local Development

If the project uses ES Modules, it MUST be served via a local HTTP server (file:// protocol will block CORS and ES modules).

1. Open VS Code.
2. Install the `Live Server` extension.
3. Right-click `public/index.html` and select "Open with Live Server".
```

## .gitignore

Every web project MUST include a `.gitignore`.

```text
# OS files
.DS_Store
Thumbs.db

# IDE files
.vscode/
*.swp

# Environment variables
.env
.env.local

# Node (If package.json is introduced later for tooling)
node_modules/
npm-debug.log
```
