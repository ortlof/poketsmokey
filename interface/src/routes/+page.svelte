<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Light from '~icons/tabler/bulb';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Reload from '~icons/tabler/reload';
    import RangeSlider from "svelte-range-slider-pips";

	type AirState = {
		air: boolean;
		power: number;

	};

	let airState: AirState = { air: false, power: 0};
	let airslider = [100, 255];

	const ws_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : '';

	const AirStateSocket = new WebSocket('ws://' + $page.url.host + '/ws/AirState' + ws_token);
	AirStateSocket.onopen = (event) => {
		AirStateSocket.send('Hello');
	};

	AirStateSocket.onmessage = (event) => {
		const message = JSON.parse(event.data);
		if (message.type == 'payload') {
			airState.air = message.payload.air;
			airState.power = message.payload.power;
 		}
	};

	onDestroy(() => AirStateSocket.close());

	type PowerState = {
		voltage: number;
		coilpw: number;
	};
	
	let powerState: PowerState = { voltage: 0, coilpw: 0};

	const ws2_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : '';

	const PowerStateSocket = new WebSocket('ws://' + $page.url.host + '/ws/powerState' + ws2_token);
	PowerStateSocket.onopen = (event) => {
		PowerStateSocket.send('Hello');
	};

	PowerStateSocket.onmessage = (event) => {
		const message = JSON.parse(event.data);
		if (message.type == 'payload') {
			powerState.voltage = message.payload.voltage;
			powerState.coilpw = message.payload.coilpw;
		}
	};

	onDestroy(() => PowerStateSocket.close());

	//https://simeydotme.github.io/svelte-range-slider-pips/?ref=madewithsvelte.com#svelte-range-slider--pips
</script>




<SettingsCard collapsible={false}>
	<span slot="title">Power</span>
	<div class="w-full">
        <div class="w-full overflow-x-auto">
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
						</div>
						<div>
							<div class="font-bold">Battery Voltage</div>
							<div class="text-sm opacity-75">
								{powerState.voltage} V
							</div>
						</div>
						<div>
							<div class="font-bold">Coil Power</div>
							<div class="text-sm opacity-75">
								{powerState.coilpw} A
							</div>
						</div>
					</div>
		</div>
	</div>
</SettingsCard>

<SettingsCard collapsible={false}>
		<div class="w-full" />
		<h1 class="text-xl font-semibold">Control</h1>
		<div class="w-full overflow-x-auto">
			<label class="label cursor-pointer">
				<span class="">air</span>
				<input
					type="checkbox"
					class="toggle toggle-primary"
					bind:checked={airState.air}
					on:change={() => {
						AirStateSocket.send(JSON.stringify(airState.air));
					}}
				/>
			</label>
			<RangeSlider float min={0} max={255} bind:value={airState.power} pips on:change={() => {
				AirStateSocket.send(JSON.stringify(airState.power));
			}}/>
		</div>
</SettingsCard>