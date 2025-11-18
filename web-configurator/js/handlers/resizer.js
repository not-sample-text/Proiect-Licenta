import { dom } from "../dom.js";

export const ResizerHandler = {
	init: () => {
		if (!dom.resizer) return;

		let isResizing = false;

		dom.resizer.addEventListener("mousedown", (e) => {
			isResizing = true;
			dom.resizer.classList.add("resizing");
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
				dom.resizer.classList.remove("resizing");
				document.body.style.cursor = "default";
			}
		});
	}
};
