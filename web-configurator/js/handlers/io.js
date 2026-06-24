import { dom } from "../dom.js";
import { configData } from "../state.js";
import { PersistenceHandler } from "../utils/storage.js";

export const IOHandler = {
    init: () => {
        // EXPORT
        dom.exportBtn.addEventListener("click", () => {
            const dataStr =
                "data:text/json;charset=utf-8," +
                encodeURIComponent(JSON.stringify(configData, null, 2));
            const downloadAnchor = document.createElement("a");
            downloadAnchor.setAttribute("href", dataStr);
            downloadAnchor.setAttribute("download", "config.json");
            document.body.appendChild(downloadAnchor);
            downloadAnchor.click();
            downloadAnchor.remove();

            // FIX: Updated alert to reflect the new Daemon architecture
            alert(
                "Config exported! Keep the file in your Downloads folder—the ApexPad Background Daemon will detect it automatically."
            );
        });

        // IMPORT
        dom.importBtn.addEventListener("click", () => dom.importInput.click());

        dom.importInput.addEventListener("change", (e) => {
            const file = e.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = (event) => {
                try {
                    const parsed = JSON.parse(event.target.result);
                    PersistenceHandler.applyConfig(parsed);
                    alert("Configuration imported successfully!");
                    PersistenceHandler.saveToLocal();
                } catch (err) {
                    alert("Error: Invalid JSON file.");
                }
            };
            reader.readAsText(file);
            dom.importInput.value = "";
        });
    }
};
