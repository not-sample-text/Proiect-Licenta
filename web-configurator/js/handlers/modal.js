import { dom } from "../dom.js";
import { configData, state } from "../state.js";
import { PersistenceHandler } from "../utils/storage.js";
import { KeyHandler } from "./key.js";
import { SidebarHandler } from "./sidebar.js";

// Track the global keyboard handler for cleanup
let globalKeydownHandler = null;
// Save modal header/footer state when showing transient prompts
let _savedModalHeaderHTML = null;
let _savedFooterHidden = false;

export const ModalHandler = {
	setupRecorder: () => {
		const recorder = document.querySelector(".shortcut-recorder");

		// Start recording
		recorder.addEventListener("click", () => {
			recorder.classList.add("recording");
			dom.modal.shortcutDisplay.innerText = "Listening...";
			recorder.focus();

			// Add global document-level listener to catch ALL keyboard events
			globalKeydownHandler = (e) => {
				// Prevent ALL default browser behavior while recording
				e.preventDefault();
				e.stopPropagation();

				// Merge any modifiers previously set via buttons with physical modifier state
				const existing = dom.modal.shortcutValue.value
					? dom.modal.shortcutValue.value.split(" + ").filter(Boolean)
					: [];
				const canonicalMods = ["Meta", "Ctrl", "Shift", "Alt"];
				const modsSet = new Set(
					existing.filter((i) => canonicalMods.includes(i))
				);
				if (e.ctrlKey) modsSet.add("Ctrl");
				if (e.shiftKey) modsSet.add("Shift");
				if (e.altKey) modsSet.add("Alt");
				if (e.metaKey) modsSet.add("Meta");

				const modsArray = canonicalMods.filter((m) => modsSet.has(m));

				// If the pressed key is a non-modifier, include it as the main key (replace any previous main)
				if (!["Control", "Shift", "Alt", "Meta"].includes(e.key)) {
					const main = e.key.toUpperCase();
					const result = modsArray.concat(main).join(" + ");
					dom.modal.shortcutValue.value = result;
					dom.modal.shortcutDisplay.innerText =
						ModalHandler.formatDisplayFromValue(result);

					// End recording when a non-modifier key is pressed
					recorder.blur();
					recorder.classList.remove("recording");
					// Remove the global listener
					document.removeEventListener("keydown", globalKeydownHandler, true);
					globalKeydownHandler = null;
				} else {
					// Only modifiers pressed: update stored value and show formatted display (symbols)
					const result = modsArray.join(" + ");
					dom.modal.shortcutValue.value = result;
					dom.modal.shortcutDisplay.innerText = result
						? ModalHandler.formatDisplayFromValue(result)
						: "Listening...";
				}
			};

			// Use capture phase (true) to catch the event before any other handlers
			document.addEventListener("keydown", globalKeydownHandler, true);
		});

		// Handle blur (user clicked away)
		recorder.addEventListener("blur", () => {
			if (recorder.classList.contains("recording")) {
				recorder.classList.remove("recording");
				if (globalKeydownHandler) {
					document.removeEventListener("keydown", globalKeydownHandler, true);
					globalKeydownHandler = null;
				}
			}
		});
	},

	// Return a label for the Meta key depending on the user's OS
	getMetaLabel: () => {
		try {
			const ua = navigator.userAgent || "";
			const platform = navigator.platform || "";
			// Show symbol for macOS and Windows, plain 'Meta' on Linux
			if (/Mac|iPhone|iPad|iPod/.test(ua) || /Mac/.test(platform)) return "⌘";
			if (/Win/.test(platform) || /Windows/.test(ua)) return "⊞";
			if (/Linux/.test(platform) || /X11/.test(ua)) return "Meta";
			return "Meta";
		} catch (e) {
			return "Meta";
		}
	},

	// Convert stored canonical value (tokens) into a user-facing display string
	formatDisplayFromValue: (val) => {
		if (!val) return "";
		const metaDisplay = ModalHandler.getMetaLabel();
		return val
			.split(" + ")
			.map((token) => (token === "Meta" ? metaDisplay : token))
			.join(" + ");
	},

	// Render modifier buttons (Ctrl, Shift, Alt, Meta/Command/Windows)
	setupModifierButtons: () => {
		const container = document.getElementById("input-container-shortcut");
		if (!container) return;

		// Avoid creating buttons multiple times
		if (container.querySelector(".modifier-buttons")) return;

		const bar = document.createElement("div");
		bar.className = "modifier-buttons";
		bar.style.display = "flex";
		bar.style.gap = "8px";
		bar.style.marginTop = "8px";

		const metaLabel = ModalHandler.getMetaLabel();
		const labels = [metaLabel, "Ctrl", "Shift", "Alt"];

		labels.forEach((label) => {
			// displayLabel is what the user sees; token is the canonical value stored/used in logic
			const displayLabel = label;
			const token =
				label === metaLabel && metaLabel !== "Meta" ? "Meta" : label;
			const btn = document.createElement("button");
			btn.type = "button";
			btn.className = "modifier-btn btn-secondary";
			btn.innerText = displayLabel;
			btn.dataset.token = token;
			btn.addEventListener("click", (ev) => {
				ev.preventDefault();
				const recorder = document.querySelector(".shortcut-recorder");
				const display = dom.modal.shortcutDisplay;
				const hidden = dom.modal.shortcutValue;

				let items = hidden.value
					? hidden.value.split(" + ").filter(Boolean)
					: [];
				const canonical = ["Meta", "Ctrl", "Shift", "Alt"];
				let main = items.find((i) => !canonical.includes(i));
				let mods = items.filter((i) => canonical.includes(i));

				// Toggle token presence
				const idx = mods.indexOf(token);
				if (idx === -1) mods.push(token);
				else mods.splice(idx, 1);

				// Order modifiers in canonical order
				mods = canonical.filter((m) => mods.includes(m));
				const newItems = main ? mods.concat(main) : mods;

				// Start recording if not active so physical key presses will be captured
				if (recorder && !recorder.classList.contains("recording")) {
					recorder.click();
				}

				hidden.value = newItems.join(" + ");
				display.innerText = hidden.value
					? ModalHandler.formatDisplayFromValue(hidden.value)
					: "Click to Record...";

				// Update visual active states for buttons
				ModalHandler.updateModifierButtons();

				if (recorder) setTimeout(() => recorder.focus(), 0);
			});
			bar.appendChild(btn);
		});

		container.appendChild(bar);
	},

	// Sync modifier button 'active' state to match the hidden shortcut value
	updateModifierButtons: () => {
		const container = document.getElementById("input-container-shortcut");
		if (!container) return;
		const hiddenVal = dom.modal.shortcutValue.value || "";
		const mods = hiddenVal
			? hiddenVal
					.split(" + ")
					.filter(Boolean)
					.filter((i) => ["Meta", "Ctrl", "Shift", "Alt"].includes(i))
			: [];
		container.querySelectorAll(".modifier-btn").forEach((btn) => {
			const token = btn.dataset.token;
			if (token) btn.classList.toggle("active", mods.includes(token));
		});
	},

	setupFilePickers: () => {
		const handleFile = (input, display) => {
			input.addEventListener("change", (e) => {
				if (e.target.files.length > 0) display.value = e.target.files[0].name;
			});
		};
		handleFile(
			document.getElementById("script-file-input"),
			dom.modal.scriptDisplay
		);
		handleFile(document.getElementById("app-file-input"), dom.modal.appInput);
	},

	open: (keyId, currentText) => {
		dom.modal.keyId.innerText = keyId;
		dom.modal.labelInput.value =
			currentText === "Unassigned" ? "" : currentText;

		dom.modal.overlay.classList.remove("hidden");
		document
			.querySelectorAll(".input-type-container")
			.forEach((el) => el.classList.add("hidden"));

		// ! FIX: Access configData.layers array properly
		const savedKeyData = configData.layers[state.activeLayerIndex].keys[keyId];
		const savedValue = savedKeyData ? savedKeyData.value : "";

		// Handle Layer 0 (FN Keys) - Only Name input is shown (default behavior)
		if (state.activeLayerIndex === 0) {
			// Intentionally empty: Layer 0 allows renaming but has no extra inputs
		} else if (state.activeLayerIndex === 1) {
			document
				.getElementById("input-container-shortcut")
				.classList.remove("hidden");
			dom.modal.shortcutDisplay.innerText = savedValue
				? ModalHandler.formatDisplayFromValue(savedValue)
				: "Click to Record...";
			dom.modal.shortcutValue.value = savedValue;
			// sync modifier button visuals to saved value
			setTimeout(() => ModalHandler.updateModifierButtons(), 0);
		} else if (state.activeLayerIndex === 2) {
			document
				.getElementById("input-container-script")
				.classList.remove("hidden");
			dom.modal.scriptDisplay.value = savedValue;
		} else if (state.activeLayerIndex === 3) {
			document.getElementById("input-container-app").classList.remove("hidden");
			dom.modal.appInput.value = savedValue;
		}
	},

	save: () => {
		const layerIdx = state.activeLayerIndex;
		const keyId = dom.modal.keyId.innerText;
		let displayName = dom.modal.labelInput.value.trim();
		let value = "";

		// ! FIX: Handle Layer 0 Saving (Nickname only)
		if (layerIdx === 0) {
			if (!displayName) displayName = configData.layers[0].keys[keyId].value; // Revert to Fxx if empty

			// Only update label. Keep existing Value and Type.
			configData.layers[0].keys[keyId].label = displayName;

			PersistenceHandler.saveToLocal();
			KeyHandler.updateFromData(layerIdx);
			SidebarHandler.render();
			ModalHandler.close();
			return;
		}

		// Logic for Layers 1, 2, 3
		if (layerIdx === 1) value = dom.modal.shortcutValue.value;
		else if (layerIdx === 2) value = dom.modal.scriptDisplay.value;
		else if (layerIdx === 3) value = dom.modal.appInput.value;

		if (!value) {
			delete configData.layers[layerIdx].keys[keyId];
			PersistenceHandler.saveToLocal();
			KeyHandler.updateFromData(layerIdx);
			SidebarHandler.render();
			ModalHandler.close();
			return;
		}

		if (value && !displayName) {
			displayName = value;
		}

		configData.layers[layerIdx].keys[keyId] = {
			label: displayName,
			value: value,
			type: layerIdx === 1 ? "SHORTCUT" : layerIdx === 2 ? "SCRIPT" : "APP"
		};

		PersistenceHandler.saveToLocal();
		KeyHandler.updateFromData(layerIdx);
		SidebarHandler.render();
		ModalHandler.close();
	},

	close: () => {
		// Clean up global keyboard listener if still active
		if (globalKeydownHandler) {
			document.removeEventListener("keydown", globalKeydownHandler, true);
			globalKeydownHandler = null;
		}

		// Remove recording state from recorder if still active
		const recorder = document.querySelector(".shortcut-recorder");
		if (recorder) {
			recorder.classList.remove("recording");
		}

		dom.modal.overlay.classList.add("hidden");
	},

	showSamplePrompt: (sampleConfig) => {
		if (!dom.modal.samplePrompt) return;
		// Show modal overlay and the sample prompt block
		// Save and replace header/footer so the prompt looks like its own modal
		const headerEl = document.querySelector(".modal-header");
		const footerEl = document.querySelector(".modal-footer");
		_savedModalHeaderHTML = headerEl ? headerEl.innerHTML : null;
		if (headerEl) headerEl.innerHTML = "<h3>Welcome</h3>";
		_savedFooterHidden = footerEl
			? footerEl.classList.contains("hidden")
			: false;
		if (footerEl) footerEl.classList.add("hidden");

		dom.modal.overlay.classList.remove("hidden");
		dom.modal.samplePrompt.classList.remove("hidden");
		// Hide other input containers to avoid confusion
		document
			.querySelectorAll(".input-type-container")
			.forEach((el) => el.classList.add("hidden"));

		// Wire buttons (ensure handlers are not duplicated)
		dom.modal.sampleLoadBtn.onclick = () => {
			try {
				PersistenceHandler.applyConfig(sampleConfig);
				PersistenceHandler.saveToLocal();
			} catch (e) {
				console.error("Failed to apply sample config", e);
			}
			ModalHandler.hideSamplePrompt();
		};
		dom.modal.sampleCancelBtn.onclick = () => {
			ModalHandler.hideSamplePrompt();
		};
	},

	hideSamplePrompt: () => {
		if (!dom.modal.samplePrompt) return;
		dom.modal.samplePrompt.classList.add("hidden");
		dom.modal.overlay.classList.add("hidden");

		// restore header/footer
		const headerEl = document.querySelector(".modal-header");
		const footerEl = document.querySelector(".modal-footer");
		if (headerEl && _savedModalHeaderHTML !== null)
			headerEl.innerHTML = _savedModalHeaderHTML;
		if (footerEl) {
			if (!_savedFooterHidden) footerEl.classList.remove("hidden");
			else footerEl.classList.add("hidden");
		}
		// Restore shortcut display if any value exists
		dom.modal.shortcutDisplay.innerText = dom.modal.shortcutValue.value
			? ModalHandler.formatDisplayFromValue(dom.modal.shortcutValue.value)
			: "Click to Record...";
		ModalHandler.updateModifierButtons();
	},

	reset: () => {
		// Clear text inputs
		dom.modal.labelInput.value = "";
		// Clear shortcut display and value
		dom.modal.shortcutValue.value = "";
		dom.modal.shortcutDisplay.innerText = "Click to Record...";

		// Clear script and app inputs
		dom.modal.scriptDisplay.value = "";
		const scriptFile = document.getElementById("script-file-input");
		if (scriptFile) scriptFile.value = null;
		dom.modal.appInput.value = "";
		const appFile = document.getElementById("app-file-input");
		if (appFile) appFile.value = null;

		// Remove recording state and global listener
		const recorder = document.querySelector(".shortcut-recorder");
		if (recorder) {
			recorder.classList.remove("recording");
			recorder.blur();
		}
		if (globalKeydownHandler) {
			document.removeEventListener("keydown", globalKeydownHandler, true);
			globalKeydownHandler = null;
		}

		// Update modifier button visuals
		ModalHandler.updateModifierButtons();
	},

	init: () => {
		dom.modal.cancelBtn.addEventListener("click", ModalHandler.close);
		dom.modal.resetBtn.addEventListener("click", ModalHandler.reset);
		dom.modal.saveBtn.addEventListener("click", ModalHandler.save);
		dom.modal.overlay.addEventListener("click", (e) => {
			if (e.target === dom.modal.overlay) ModalHandler.close();
		});
		ModalHandler.setupRecorder();
		ModalHandler.setupFilePickers();
		ModalHandler.setupModifierButtons();
	}
};
