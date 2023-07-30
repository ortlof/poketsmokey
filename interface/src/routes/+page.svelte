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

	let airvalue = [200];
	let coilvalue = [0];
	let burntime = [0];
	let interval = [0];

	let AirOn = false;
	let CoilOn = false;
	let fireOn = false;
	let IntervalOn = false;

	async function getAirstate() {
		try {
			const response = await fetch('/rest/airState', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			const air = await response.json();
			AirOn = air.air;
			airvalue = [parseFloat(air.airpower)];
			CoilOn = air.coil;
			coilvalue = [parseFloat(air.coilpower)];
		    IntervalOn = air.intervalstate;
			interval = [parseFloat(air.interval)];
			burntime = [parseFloat(air.burntime)];
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postAirstate() {
		try {

			const response = await fetch('/rest/airState', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ air: AirOn, airpower: Number(airvalue), coil: CoilOn, coilpower: Number(coilvalue), intervalstate: IntervalOn, interval: Number(interval), burntime: Number(burntime) })
			});
			if (response.status == 200) {
				//notifications.success('Light state updated.', 3000);
				const air = await response.json();
				AirOn = air.air;
				airvalue = [parseFloat(air.airpower)];
				CoilOn = air.coil;
				coilvalue = [parseFloat(air.coilpower)];
				IntervalOn = air.intervalstate;
				interval = [parseFloat(air.interval)];
				burntime = [parseFloat(air.burntime)];				
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}


	type powerState = {
		voltage: number;
		coilpw: number;
	};
	
	let powerState: powerState = { voltage: 0, coilpw: 0};

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

	
  let intervalId = null;

  function handleMouseDown() {
    intervalId = setInterval(() => {
      console.log('Button is being held down');
	  AirOn = true;
	  CoilOn = true;
	  fireOn = true;
	  postAirstate();
    }, 1000);  // Every 1 second
  }

  function handleMouseUp() {
    clearInterval(intervalId);
    intervalId = null;
	AirOn = false;
	CoilOn = false;
	fireOn = false;
	postAirstate();
  }

  function handleMouseLeave() {
	if(fireOn == true){
    clearInterval(intervalId);
    intervalId = null;
	AirOn = false;
	CoilOn = false;
	fireOn = false;
	postAirstate();
	}
  }

  onMount(() => {
	getAirstate();
    return () => {
      if (intervalId) {
        clearInterval(intervalId);
      }
    };
  });

</script>


<SettingsCard collapsible={false}>
	<div class="w-full">
        <div class="w-full overflow-x-auto">
					<div class="rounded-box flex items-center space-x-3 px-4 py-2">
						<div class="left">
							<div class="font-bold">Battery Voltage</div>
							<div class="text-sm opacity-75">
								{powerState.voltage} V
							</div>
						</div>
						<div class="right">
							<div class="font-bold">Coil Power</div>
							<div class="text-sm opacity-75">
								{powerState.coilpw} A
							</div>
						</div>
					</div>
		</div>
        <div class="w-full overflow-x-auto ">
			<div class="center">
					<div class="rounded-box flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto">
						<button class="btn btn-primary inline-flex flex-none items-center"   
							on:mousedown={handleMouseDown} on:mouseup={handleMouseUp} on:mouseleave={handleMouseLeave}> 
						Manual Fire</button>
						</div>
					</div>
			</div>
		</div>
		
	</div>

		<div class="w-full" />
		<h1 class="text-xl font-semibold">Smoke Control</h1>
		<div class="w-full overflow-x-auto">
			<label class="label cursor-pointer">
				<span class="">Air: {airvalue} %</span>
				<input
					type="checkbox"
					class="toggle toggle-primary"
					bind:checked={AirOn}			
					on:change={postAirstate}
				/>
			</label>
			
			<RangeSlider bind:values={airvalue} min={0} max={100} pips on:stop={postAirstate}/>
        </div>
		<div class="w-full overflow-x-auto">
			<label class="label cursor-pointer">
				<span class="">Coil: {coilvalue} %</span>
				<input
					type="checkbox"
					class="toggle toggle-primary"
					bind:checked={CoilOn}			
					on:change={postAirstate}
				/>
			</label>
			<RangeSlider bind:values={coilvalue} min={0} max={100} pips on:stop={postAirstate}/>
		</div>

		<h1 class="text-xl font-semibold">Timer Control</h1>
		<div class="w-full overflow-x-auto">
			<label class="label cursor-pointer">
				<span class="">Smoke Burn Time Sek: {burntime}</span>
			</label>
			<RangeSlider bind:values={burntime} min={0} max={10}  pips on:stop={postAirstate}/>
        </div>
		<div class="w-full overflow-x-auto">
			<label class="label cursor-pointer">
				<span class="">Interval Sek: {interval}</span>
				<input
				type="checkbox"
				class="toggle toggle-primary"
				bind:checked={IntervalOn}			
				on:change={postAirstate}
			/>
			</label>
			<RangeSlider bind:values={interval} min={0} max={60}  pips on:stop={postAirstate} size="lg"/>
		</div>
</SettingsCard>
<style>
	.center {
	display: flex;
	align-items: center;
	justify-content: center;
	}
	.btn {
	height: 150px; 
	width: 150px;
	}
	.right {
		margin-left: auto;
	}
	.left {
		margin-right: auto;
	}
</style>