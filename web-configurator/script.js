document.addEventListener("DOMContentLoaded", () => {
	// --- Global State ---
	const state = {
		activeLayerIndex: -1,
		isKnobNavMode: false,
		volume: 50,
		knobRotation: 0
	};

	// --- DATA MODEL INITIALIZATION ---
	// Helper to generate the Immutable Layer 1 (F13-F24)
	// Grid is 3 columns x 4 rows. Order: Row 0 (C0-C2), Row 1...
	const layer1Keys = {};
	let fCount = 13;
	// Rows 0 to 3
	for (let r = 0; r < 4; r++) {
		// Columns 0 to 2
		for (let c = 0; c < 3; c++) {
			layer1Keys[`C${c}R${r}`] = {
				label: `F${fCount}`,
				value: "",
				type: "KEY"
			};
			fCount++;
		}
	}

	const configData = [
		{ id: 0, name: "FN Keys", keys: layer1Keys }, // Pre-filled Layer 1
		{ id: 1, name: "Shortcuts", keys: {} }, // Layer 2 (Starts Empty)
		{ id: 2, name: "Commands", keys: {} }, // Layer 3 (Starts Empty)
		{ id: 3, name: "Launcher", keys: {} } // Layer 4 (Starts Empty)
	];

	// --- Selectors ---
	const dom = {
		macropad: document.querySelector(".macropad-container"),
		title: document.getElementById("layer-title"),
		sidebarHeaders: document.querySelectorAll(".sidebar .category-header"),
		sidebarItems: document.querySelectorAll(".sidebar .key-list li"),
		keys: document.querySelectorAll(".macropad-container .key"),
		oled: {
			layerName: document.getElementById("oled-layer-name"),
			mainText: document.getElementById("oled-display-text")
		},
		knob: {
			container: document.querySelector(".rotary-encoder"),
			element: document.querySelector(".knob")
		},
		modal: {
			overlay: document.getElementById("config-modal"),
			keyId: document.getElementById("modal-key-id"),
			labelInput: document.getElementById("key-label"),
			shortcutDisplay: document.getElementById("shortcut-display"),
			shortcutValue: document.getElementById("shortcut-value"),
			scriptDisplay: document.getElementById("script-path-display"),
			appInput: document.getElementById("app-path-input"),
			cancelBtn: document.getElementById("btn-cancel"),
			saveBtn: document.getElementById("btn-save")
		},
		exportBtn: document.getElementById("btn-export-json")
	};

	const layerNameMap = {
		"Layer 1 - F13-F24": "FN KEYS",
		"Layer 2 - Shortcuts": "SHORTCUTS",
		"Layer 3 - Commands": "COMMANDS",
		"Layer 4 - Open Apps": "LAUNCHER"
	};

	// --- HANDLERS ---

	const OLEDHandler = {
		updateHeader: (fullName, index) => {
			const shortName = layerNameMap[fullName] || "LAYER " + (index + 1);
			dom.oled.layerName.innerText = shortName;
			dom.oled.mainText.innerText = "READY";
		},
		updateMain: (text) => {
			// Logic: If text is "Unassigned", show it fully. Otherwise truncate.
			if (text === "Unassigned") {
				dom.oled.mainText.innerText = "UNASSIGNED";
			} else {
				dom.oled.mainText.innerText =
					text.length > 12 ? text.substring(0, 10) + ".." : text;
			}
		}
	};

	const KeyHandler = {
		resetAll: () => {
			dom.keys.forEach((key) => {
				key.innerText = "Unassigned";
				key.classList.add("unassigned-key");
			});
		},
		updateFromData: (layerIndex) => {
			const layerKeys = configData[layerIndex].keys;

			dom.keys.forEach((key) => {
				const id = key.getAttribute("data-id");

				// Reset style first
				key.classList.remove("unassigned-key");

				if (layerKeys[id]) {
					// Case 1: Key is assigned
					key.innerText = layerKeys[id].label;
				} else {
					// Case 2: Key is empty -> Show "Unassigned"
					key.innerText = "Unassigned";
					key.classList.add("unassigned-key");
				}
			});
		},
		simulatePress: (keyId) => {
			const key = document.querySelector(
				`.macropad-container .key[data-id="${keyId}"]`
			);
			if (key) {
				key.classList.add("pressed-simulation");
				setTimeout(() => key.classList.remove("pressed-simulation"), 200);
			}
		}
	};

	const SidebarHandler = {
		resetViews: () => {
			document.querySelectorAll(".sidebar .key-list").forEach((el) => {
				el.classList.remove("open", "read-only-list");
			});
			document
				.querySelectorAll(".sidebar .category-icon")
				.forEach((el) => el.classList.remove("open"));
		},
		activateLayer: (index, header) => {
			const keyList = header.nextElementSibling;
			const icon = header.querySelector(".category-icon");
			const name = header.querySelector(".category").innerText;

			keyList.classList.add("open");
			icon.classList.add("open");
			dom.title.textContent = name;

			state.activeLayerIndex = index;
			OLEDHandler.updateHeader(name, index);

			// Update Grid Keys
			KeyHandler.updateFromData(index);

			if (state.activeLayerIndex === 0) {
				dom.macropad.classList.add("read-only");
				keyList.classList.add("read-only-list");
			} else {
				dom.macropad.classList.remove("read-only");
			}
		}
	};

	const ModalHandler = {
		setupRecorder: () => {
			const recorder = document.querySelector(".shortcut-recorder");
			recorder.addEventListener("click", () => {
				recorder.classList.add("recording");
				dom.modal.shortcutDisplay.innerText = "Listening...";
			});
			recorder.addEventListener("keydown", (e) => {
				e.preventDefault();
				const keys = [];
				if (e.ctrlKey) keys.push("Ctrl");
				if (e.shiftKey) keys.push("Shift");
				if (e.altKey) keys.push("Alt");
				if (e.metaKey) keys.push("Meta");
				if (!["Control", "Shift", "Alt", "Meta"].includes(e.key)) {
					keys.push(e.key.toUpperCase());
				}
				const result = keys.join(" + ");
				dom.modal.shortcutDisplay.innerText = result;
				dom.modal.shortcutValue.value = result;

				if (!["Control", "Shift", "Alt", "Meta"].includes(e.key)) {
					recorder.blur();
					recorder.classList.remove("recording");
				}
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

			// If the key is "Unassigned", leave the input blank for a fresh start
			dom.modal.labelInput.value =
				currentText === "Unassigned" ? "" : currentText;

			dom.modal.overlay.classList.remove("hidden");
			document
				.querySelectorAll(".input-type-container")
				.forEach((el) => el.classList.add("hidden"));

			const savedKeyData = configData[state.activeLayerIndex].keys[keyId];
			const savedValue = savedKeyData ? savedKeyData.value : "";

			if (state.activeLayerIndex === 1) {
				document
					.getElementById("input-container-shortcut")
					.classList.remove("hidden");
				dom.modal.shortcutDisplay.innerText =
					savedValue || "Click to Record...";
				dom.modal.shortcutValue.value = savedValue;
			} else if (state.activeLayerIndex === 2) {
				document
					.getElementById("input-container-script")
					.classList.remove("hidden");
				dom.modal.scriptDisplay.value = savedValue;
			} else if (state.activeLayerIndex === 3) {
				document
					.getElementById("input-container-app")
					.classList.remove("hidden");
				dom.modal.appInput.value = savedValue;
			}
		},
		save: () => {
			const layerIdx = state.activeLayerIndex;
			const keyId = dom.modal.keyId.innerText;
			const displayName = dom.modal.labelInput.value || "Unassigned"; // Revert if empty
			let value = "";

			if (layerIdx === 1) value = dom.modal.shortcutValue.value;
			else if (layerIdx === 2) value = dom.modal.scriptDisplay.value;
			else if (layerIdx === 3) value = dom.modal.appInput.value;

			// If user cleared the name, delete the config for this key
			if (displayName === "Unassigned" || displayName === "") {
				delete configData[layerIdx].keys[keyId];
			} else {
				configData[layerIdx].keys[keyId] = {
					label: displayName,
					value: value,
					type: layerIdx === 1 ? "SHORTCUT" : layerIdx === 2 ? "SCRIPT" : "APP"
				};

				// --- NEW: Save to LocalStorage ---
				PersistenceHandler.saveToLocal();
			}

			// Refresh Grid
			KeyHandler.updateFromData(layerIdx);
			ModalHandler.close();
		},
		close: () => {
			dom.modal.overlay.classList.add("hidden");
		},
		init: () => {
			dom.modal.cancelBtn.addEventListener("click", ModalHandler.close);
			dom.modal.saveBtn.addEventListener("click", ModalHandler.save);
			dom.modal.overlay.addEventListener("click", (e) => {
				if (e.target === dom.modal.overlay) ModalHandler.close();
			});
			ModalHandler.setupRecorder();
			ModalHandler.setupFilePickers();
		}
	};

	const PersistenceHandler = {
		// Key for LocalStorage
		STORAGE_KEY: "macropad_config_v1",

		// 1. Save current state to Browser Memory
		saveToLocal: () => {
			try {
				localStorage.setItem(
					PersistenceHandler.STORAGE_KEY,
					JSON.stringify(configData)
				);
				console.log("Auto-saved to LocalStorage");
			} catch (e) {
				console.warn("LocalStorage failed (likely disabled):", e);
			}
		},

		// 2. Load from Browser Memory
		loadFromLocal: () => {
			const saved = localStorage.getItem(PersistenceHandler.STORAGE_KEY);
			if (saved) {
				try {
					const parsed = JSON.parse(saved);
					PersistenceHandler.applyConfig(parsed);
					console.log("Restored from LocalStorage");
					return true;
				} catch (e) {
					console.error("Corrupt LocalStorage data", e);
					return false;
				}
			}
			return false;
		},

		// 3. Import from File
		initImport: () => {
			const fileInput = document.getElementById("import-file-input");
			const importBtn = document.getElementById("btn-import-json");

			// Link button to hidden input
			importBtn.addEventListener("click", () => fileInput.click());

			// Handle File Selection
			fileInput.addEventListener("change", (e) => {
				const file = e.target.files[0];
				if (!file) return;

				const reader = new FileReader();
				reader.onload = (event) => {
					try {
						const parsed = JSON.parse(event.target.result);
						PersistenceHandler.applyConfig(parsed);
						alert("Configuration imported successfully!");

						// Also save this import to local storage immediately
						PersistenceHandler.saveToLocal();
					} catch (err) {
						alert("Error: Invalid JSON file.");
						console.error(err);
					}
				};
				reader.readAsText(file);

				// Reset input so you can load the same file again if needed
				fileInput.value = "";
			});
		},

		// 4. Helper to Apply Data & Refresh UI
		applyConfig: (newData) => {
			// Validate basic structure (Array of 4 layers)
			if (!Array.isArray(newData) || newData.length !== 4) {
				throw new Error("Invalid Config Structure");
			}

			// Update the Global Data Model
			// We use splice to replace contents without breaking the const reference
			configData.splice(0, configData.length, ...newData);

			// Refresh UI
			// If a layer is active, refresh grid. If not, it will refresh on next click.
			if (state.activeLayerIndex !== -1) {
				KeyHandler.updateFromData(state.activeLayerIndex);
			}
		}
	};

	const ExportHandler = {
		init: () => {
			dom.exportBtn.addEventListener("click", () => {
				const dataStr =
					"data:text/json;charset=utf-8," +
					encodeURIComponent(JSON.stringify(configData, null, 2));
				const downloadAnchor = document.createElement("a");

				downloadAnchor.setAttribute("href", dataStr);
				// ! IMPORTANT: Standardize the filename
				downloadAnchor.setAttribute("download", "config.json");

				document.body.appendChild(downloadAnchor);
				downloadAnchor.click();
				downloadAnchor.remove();

				// * UX Hint
				alert(
					"Please save this file to your 'Documents/Macropad_Config' folder so the Host Listener can find it!"
				);
			});
		}
	};

	const ResizerHandler = {
		init: () => {
			const resizer = document.querySelector(".resizer");
			let isResizing = false;
			if (!resizer) return;

			resizer.addEventListener("mousedown", (e) => {
				isResizing = true;
				resizer.classList.add("resizing");
				document.body.style.cursor = "col-resize";
				e.preventDefault();
			});
			window.addEventListener("mousemove", (e) => {
				if (!isResizing) return;
				let newWidth = e.clientX;
				if (newWidth < 200) newWidth = 200;
				if (newWidth > 600) newWidth = 600;
				document.documentElement.style.setProperty(
					"--sidebar-width",
					`${newWidth}px`
				);
			});
			window.addEventListener("mouseup", () => {
				if (isResizing) {
					isResizing = false;
					resizer.classList.remove("resizing");
					document.body.style.cursor = "default";
				}
			});
		}
	};

	const KnobHandler = {
		triggerRotation: (direction) => {
			const DETENTS = 20;
			const DEGREES_PER_CLICK = 360 / DETENTS;
			const VOL_STEP = 5;

			if (direction === "right") state.knobRotation += DEGREES_PER_CLICK;
			else state.knobRotation -= DEGREES_PER_CLICK;

			dom.knob.element.style.transform = `rotate(${state.knobRotation}deg)`;

			if (state.isKnobNavMode) {
				let nextIndex;
				if (direction === "right") {
					nextIndex = state.activeLayerIndex + 1;
					if (nextIndex >= dom.sidebarHeaders.length) nextIndex = 0;
				} else {
					nextIndex = state.activeLayerIndex - 1;
					if (nextIndex < 0) nextIndex = dom.sidebarHeaders.length - 1;
				}
				dom.sidebarHeaders[nextIndex].click();
				setTimeout(() => OLEDHandler.updateMain("<< SCROLL >>"), 50);
			} else {
				if (direction === "right")
					state.volume = Math.min(100, state.volume + VOL_STEP);
				else state.volume = Math.max(0, state.volume - VOL_STEP);
				OLEDHandler.updateMain(`VOL: ${state.volume}%`);
			}
		},
		init: () => {
			let isDragging = false;
			let startX = 0;
			let hasMoved = false;

			dom.knob.element.addEventListener("mousedown", (e) => {
				isDragging = true;
				hasMoved = false;
				startX = e.clientX;
				e.preventDefault();
			});

			window.addEventListener("mousemove", (e) => {
				if (!isDragging) return;
				const deltaX = e.clientX - startX;
				if (deltaX > 15) {
					KnobHandler.triggerRotation("right");
					startX = e.clientX;
					hasMoved = true;
				} else if (deltaX < -15) {
					KnobHandler.triggerRotation("left");
					startX = e.clientX;
					hasMoved = true;
				}
			});

			window.addEventListener("mouseup", () => {
				if (!isDragging) return;
				isDragging = false;
				if (!hasMoved) {
					state.isKnobNavMode = !state.isKnobNavMode;
					if (state.isKnobNavMode) {
						dom.knob.container.classList.add("nav-mode");
						OLEDHandler.updateMain("LAYER SELECT");
					} else {
						dom.knob.container.classList.remove("nav-mode");
						OLEDHandler.updateMain(`VOL: ${state.volume}%`);
					}
					dom.knob.element.style.transform = `rotate(${state.knobRotation}deg) scale(0.96)`;
					setTimeout(
						() =>
							(dom.knob.element.style.transform = `rotate(${state.knobRotation}deg)`),
						100
					);
				}
			});

			dom.knob.element.addEventListener("wheel", (e) => {
				e.preventDefault();
				KnobHandler.triggerRotation(e.deltaY < 0 ? "right" : "left");
			});
		}
	};

	// --- Event Binding ---
	dom.sidebarHeaders.forEach((header, index) => {
		header.addEventListener("click", () => {
			const keyList = header.nextElementSibling;
			if (keyList.classList.contains("open")) return;
			SidebarHandler.resetViews();
			SidebarHandler.activateLayer(index, header);
		});
	});

	dom.sidebarItems.forEach((item) => {
		item.addEventListener("click", () => {
			if (state.activeLayerIndex <= 0) return;
			const parts = item.innerText.split("]: ");
			if (parts.length < 2) return;
			const id = parts[0].replace("[", "");
			const label = parts.slice(1).join("]: ");
			OLEDHandler.updateMain(label);
			KeyHandler.simulatePress(id);
			setTimeout(() => ModalHandler.open(id, label), 500);
		});
	});

	dom.keys.forEach((key) => {
		key.addEventListener("click", () => {
			const text = key.innerText;
			OLEDHandler.updateMain(text); // Will show "UNASSIGNED" if text is Unassigned
			if (state.activeLayerIndex > 0) {
				ModalHandler.open(key.getAttribute("data-id"), text);
			}
		});
	});

	// --- Init ---
	ModalHandler.init();
	KnobHandler.init();
	ExportHandler.init();
	ResizerHandler.init();
	PersistenceHandler.initImport(); // Setup the Import listener

	// 1. Try to load from LocalStorage
	const hasData = PersistenceHandler.loadFromLocal();

	// 2. Auto-start
	// If we have data, the UI is already updated in memory,
	// clicking the header will render it to the screen.
	if (dom.sidebarHeaders.length > 0) {
		dom.sidebarHeaders[0].click();
	}
});
