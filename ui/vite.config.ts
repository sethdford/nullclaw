import { defineConfig } from "vite";

export default defineConfig({
  build: {
    outDir: "dist",
    emptyOutDir: true,
    chunkSizeWarningLimit: 800,
    rollupOptions: {
      output: {
        manualChunks: {
          vendor: [
            "lit",
            "lit/decorators.js",
            "lit/directives/class-map.js",
            "lit/directives/style-map.js",
          ],
          markdown: ["marked", "dompurify"],
        },
      },
    },
  },
  server: {
    proxy: {
      "/ws": {
        target: "http://localhost:3000",
        ws: true,
      },
    },
  },
});
