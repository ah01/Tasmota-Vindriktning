----

📣 **Tento projekt je mrtvý. Funkcionalita je poměrně jednoduchá. A šlo by ji napsat v [Berry](https://tasmota.github.io/docs/Berry/#creating-a-tasmota-driver). Není potřeba udržovat nativní Tasmota driver.**

----


# Ikea Vindriktning a Tasmota

V tomto repu. je vývojová verze úpravy [Tasmota]() FW, tak aby mohl být použit na PCB [LaskaKit ESP-VINDRIKTNING ESP-32 I2C](https://www.laskakit.cz/laskakit-esp-vindriktning-esp-32-i2c/).

Cílem je dostat změny přímo do zdrojových kódu Tasmota. Do té doby je podpora ve forku [ah01/Tasmota](https://github.com/ah01/Tasmota).

## Proč?

Tasmota základní podporu pro sensor používaný v **Ikea Vindriktning** již [obsahuje](https://blakadder.com/vindriktning-tasmota/). Bohužel, ale jen v režimu, kdy odposlouchává existující elektroniku. Neumí sensor ovládat. Což je nezbytné, pokud chceme vyměnit původní elektroniku úplně.

## Připravený build Tasmota FW

V sekci **[Releases](https://github.com/ah01/Tasmota-Vindriktning/releases)** najdete připravené buildy Tasmota FW pro **LaskaKit ESP-VINDRIKTNING** s zkompilovanou podporou všech potřebných sensorů.

[**Návod na otestování ZDE.**](build.md)

## Vývoj FW

V adresáři `fw/tasmota-mock` je mock tasmota Sensor API, tak aby bylo možné driver `xsns_91_vindriktning.ino` vyvyjite samostatně bez nutnosti kompilovat celý projekt Tasmota.
