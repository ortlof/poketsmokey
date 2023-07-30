import { sveltekit } from '@sveltejs/kit/vite';
import type { UserConfig } from 'vite';
import Icons from 'unplugin-icons/vite';
import viteLittleFS from './vite-plugin-littlefs';

const config: UserConfig = {
	plugins: [
		sveltekit(),
		Icons({
			compiler: 'svelte'
		}),
		// Shorten file names for LittleFS 32 char limit
		viteLittleFS()
	],
	mode: 'development',
	server: {
		proxy: {
			// Proxying REST: http://localhost:5173/rest/bar -> http://192.168.1.83/rest/bar
			'/rest': {
				target: 'http://192.168.2.153',
				changeOrigin: true,
				/*configure: (proxy, _options) => {
					proxy.on("error", (err, _req, _res) => {
					  console.log("proxy error", err);
					});
					proxy.on("proxyReq", (proxyReq, req, _res) => {
					  console.log(
						"Sending Request:",
						req.method,
						req.url,
						" => TO THE TARGET =>  ",
						proxyReq.method,
						proxyReq.protocol,
						proxyReq.host,
						proxyReq.path,
						JSON.stringify(proxyReq.getHeaders()),
					  );
					});
					proxy.on("proxyRes", (proxyRes, req, _res) => {
					  console.log(
						"Received Response from the Target:",
						proxyRes.statusCode,
						req.url,
						JSON.stringify(proxyRes.headers),
					  );
					});
				},*/
			},
			// Proxying websockets ws://localhost:5173/ws -> ws://192.168.1.83/ws
			'/ws': {
				target: 'ws://192.168.2.153',
				changeOrigin: true,
				ws: true
			}
		}
	}
};

export default config;
