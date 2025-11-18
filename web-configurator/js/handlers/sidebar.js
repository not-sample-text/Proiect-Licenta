import { dom } from "../dom.js";
import { configData, state } from "../state.js";
import { OLEDHandler } from "./oled.js";
import { KeyHandler } from "./key.js";
import { ModalHandler } from "./modal.js";
import { LightingHandler } from "./lighting.js";

export const SidebarHandler = {
	render: () => {
		configData.layers.forEach((layer, index) => {
			const listContainer = document.getElementById(`layer-list-${index}`);
			if (!listContainer) return;
			listContainer.innerHTML = "";

			for (let r = 0; r < 4; r++) {
				for (let c = 0; c < 3; c++) {
					const keyId = `C${c}R${r}`;
					const keyData = layer.keys[keyId];
					const li = document.createElement("li");

					li.dataset.id = keyId;
					li.dataset.layer = index;

					if (keyData && keyData.value) {
						li.classList.add("sidebar-item-configured");
						let iconClass = "ti-keyboard";
						if (index === 2) iconClass = "ti-file-code";
						if (index === 3) iconClass = "ti-app-window";
						li.innerHTML = `<i class="ti ${iconClass} sidebar-item-icon"></i><span>${keyData.label}</span>`;
					} else {
						li.classList.add("sidebar-item-unconfigured");
						li.innerHTML = `<i class="ti ti-plus sidebar-item-icon" style="opacity:0.3"></i><span>[${keyId}]</span>`;
					}

					li.addEventListener("click", () => {
						if (state.activeLayerIndex !== index) {
							const headers = document.querySelectorAll(
								".sidebar .category-header[data-layer]"
							);
							if (headers[index]) headers[index].click();
						}
						KeyHandler.simulatePress(keyId);
						const label =
							keyData && keyData.label ? keyData.label : "Unassigned";
						OLEDHandler.updateMain(label);
						// Allow clicking items in sidebar to open modal even on Layer 0
						setTimeout(() => ModalHandler.open(keyId, label), 300);
					});
					listContainer.appendChild(li);
				}
			}
		});

		const settingsBtn = document.getElementById("sidebar-settings-btn");
		if (settingsBtn) {
			settingsBtn.addEventListener("click", () => {
				SidebarHandler.switchToSettings();
			});
		}

		const backBtn = document.getElementById("back-to-layers-btn");
		if (backBtn) {
			backBtn.addEventListener("click", () => {
				const targetIndex =
					state.lastActiveLayerIndex !== undefined &&
					state.lastActiveLayerIndex !== -1
						? state.lastActiveLayerIndex
						: 0;
				const headers = document.querySelectorAll(
					".sidebar .category-header[data-layer]"
				);
				if (headers[targetIndex]) headers[targetIndex].click();
			});
		}
	},

	toggleLayer: (index, header) => {
		SidebarHandler.switchView("KEYS");
		document.getElementById("sidebar-layers-view").classList.remove("hidden");
		document.getElementById("sidebar-controls-view").classList.add("hidden");

		const keyList = header.nextElementSibling;
		const icon = header.querySelector(".category-icon");
		const name = header.querySelector(".category").innerText;

		document
			.querySelectorAll(".sidebar .key-list")
			.forEach((el) => el.classList.remove("open", "read-only-list"));
		document
			.querySelectorAll(".sidebar .category-icon")
			.forEach((el) => el.classList.remove("open"));

		keyList.classList.add("open");
		icon.classList.add("open");
		dom.title.textContent = name;

		state.activeLayerIndex = index;
		OLEDHandler.updateHeader(name, index);
		KeyHandler.updateFromData(index);

		// ! FIX: Removed Read-Only class logic. All layers are interactive.
		dom.macropad.classList.remove("read-only");
	},

	toggleSidebar: () => {
		dom.sidebar.classList.toggle("collapsed");
		dom.hamburger.classList.toggle("active");
	},

	switchToSettings: () => {
		if (state.activeLayerIndex !== -1) {
			state.lastActiveLayerIndex = state.activeLayerIndex;
		}
		document.getElementById("sidebar-layers-view").classList.add("hidden");
		document.getElementById("sidebar-controls-view").classList.remove("hidden");
		document
			.querySelectorAll(".sidebar .key-list")
			.forEach((el) => el.classList.remove("open"));
		document
			.querySelectorAll(".sidebar .category-icon")
			.forEach((el) => el.classList.remove("open"));
		dom.title.innerText = "LIGHTING & SETTINGS";
		SidebarHandler.switchView("LIGHTING");
		state.activeLayerIndex = -1;
		OLEDHandler.updateMain("SETTINGS");
	},

	switchView: (viewName) => {
		const keyContainer = document.querySelector(".macropad-container");
		const lightingView = document.querySelector(".lighting-view");
		if (viewName === "LIGHTING") {
			keyContainer.classList.add("hidden");
			lightingView.classList.remove("hidden");
		} else {
			keyContainer.classList.remove("hidden");
			lightingView.classList.add("hidden");
		}
	}
};
