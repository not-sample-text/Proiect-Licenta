import { dom } from "../dom.js";
import { state } from "../state.js";
import { OLEDHandler } from "./oled.js";

export const KnobHandler = {
	triggerRotation: (direction) => {
		const DEGREES_PER_CLICK = 360 / 20;
		const VOL_STEP = 5;

		if (direction === "right") state.knobRotation += DEGREES_PER_CLICK;
		else state.knobRotation -= DEGREES_PER_CLICK;

		dom.knob.element.style.transform = `rotate(${state.knobRotation}deg)`;

		if (state.isKnobNavMode) {
			let nextIndex;
			if (direction === "right") {
				nextIndex = state.activeLayerIndex + 1;
				if (nextIndex >= 4) nextIndex = 0;
			} else {
				nextIndex = state.activeLayerIndex - 1;
				if (nextIndex < 0) nextIndex = 3;
			}
			// Trigger click on the static headers
			document.querySelectorAll(".sidebar .category-header")[nextIndex].click();
			setTimeout(() => OLEDHandler.updateMain("<< SCROLL >>"), 50);
		} else {
			if (direction === "right")
				state.volume = Math.min(100, state.volume + VOL_STEP);
			else state.volume = Math.max(0, state.volume - VOL_STEP);
			OLEDHandler.updateMain(`VOL: ${state.volume}%`);
		}
	},

	init: () => {
		dom.knob.element.addEventListener("wheel", (e) => {
			e.preventDefault();
			KnobHandler.triggerRotation(e.deltaY < 0 ? "right" : "left");
		});

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
			if (e.clientX - startX > 15) {
				KnobHandler.triggerRotation("right");
				startX = e.clientX;
				hasMoved = true;
			} else if (e.clientX - startX < -15) {
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
	}
};
