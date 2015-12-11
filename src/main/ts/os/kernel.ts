///ts:ref=Bios.d.ts
/// <reference path="../bios/Bios.d.ts"/> ///ts:ref:generated
///ts:ref=Components.d.ts
/// <reference path="../components/Components.d.ts"/> ///ts:ref:generated
///ts:ref=kernel.d.ts
/// <reference path="./usr/kernel/kernel.d.ts"/> ///ts:ref:generated

import ScreenComponent = components.ScreenComponent;
import GPUComponent = components.GPUComponent;
var module:Module;
var require:(file:string)=>any;

(function () {

  //Init module for the code below
  module = {
    exports: null
  };
  //Load the require() function
  require = (function () {
    let handle = $bios.bootFS.open('/usr/kernel/require.js', 'r');
    let src = '';
    let read:string;
    while (read = $bios.bootFS.read(handle, 512)) src += read;
    $bios.bootFS.close(handle);
    return $bios.compile('requre.js', src);
  })();

  let gpu:GPUComponent = $bios.component.first('gpu');
  let screen:ScreenComponent = $bios.component.first('screen');
  gpu.bind(screen.uuid);
  gpu.setBackground(0x000000);
  gpu.setForeground(0xFFFFFF);
  let size = gpu.getResolution();
  gpu.fill(1, 1, size[0] + 1, size[1] + 1, ' ');
  gpu.set(1, 1, 'Hello, Minecraft!');

  while (true) {
    let sig = $bios.computer.signal();
    if (sig) {
      gpu.copy(1, 1, size[0] + 1, size[1], 0, 1);
      gpu.fill(1, 1, size[0] + 1, 1, ' ');
      gpu.set(1, 1, sig.name + ':' + sig.args);
    }
    $bios.computer.sleep(0.05);
  }
})();