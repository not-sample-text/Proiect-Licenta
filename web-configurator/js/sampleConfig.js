export const sampleConfig = {
	layers: [
		{
			id: 0,
			name: "FN Keys",
			keys: {}
		},
		{
			id: 1,
			name: "Shortcuts",
			keys: {
				C0R0: { label: "New Tab", value: "Ctrl + T", type: "SHORTCUT" },
				C1R0: { label: "New Window", value: "Ctrl + N", type: "SHORTCUT" },
				C2R0: { label: "Close Tab", value: "Ctrl + W", type: "SHORTCUT" }
			}
		},
		{
			id: 2,
			name: "Commands",
			keys: {}
		},
		{
			id: 3,
			name: "Launcher",
			keys: {}
		}
	],
	lighting: {
		mode: "SOLID",
		brightness: 128,
		speed: 10,
		color: "#00e5ff"
	}
};
