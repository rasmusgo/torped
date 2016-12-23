# Torped

_Torped_ is slang for _hitman_ in Swedish. Like many others who started programming as a kid, I dreamed about creating the perfect game. My brainchild was named _Torped_ and has guided me in my endeavors in the world of programming. It made it clear to me that I would need to learn a lot of math and a lot of programming in order to be able to create the game that I wanted. It has been a guiding star during my childhood and early adulthood. I wouldn't be the person I am today if it wasn't for this game, even though it is nowhere near completion yet. This repository can be seen as a revival of my efforts or perhaps CPR for my childhood dreams.

The game might be described as a first person physics sandbox game. The player character and his surroundings are simulated in a finer degree than in most games. It is common practice to simulate characters in games as cylinders or capsules fixed in upright poses sliding on perfectly flat floors and against perfectly flat walls in a mostly static environment. The characters are not propelled by legs, the legs are added as graphical afterthought to make things look better. In Torped on the other hand, the characters interact with the world in a delicate way. They walk with legs and feet. They pick things up with their hands. Running into a wall at full speed is not a pleasant experience in the real world and doing so in Torped will have a similar dramatic effect.


## Physics
The game should simulate the world in a consistent manner. Realism is important but consistency and predictability is more important than realism. Energy preservation is the expected behavior. Things rarely appear out of nothingness and they do not disappear easily either. There is no magic inventory where items are aligned as icons in a rectangular grid. They are always simulated in three dimensions. They do not change form when picked up or dropped.

The player character is simulated as a soft body with a deformable trunk with limbs attached. Standing up is a balancing act just like real life. Most of the balancing is done by the computer but the player needs to keep in mind that there is an underlying physics simulation that will make the character fall over if the balancing act becomes too hard. The player character cannot accelerate without interacting with the environment. This means you cannot adjust your velocity in mid-air.


## Graphics
The focus of graphics in the game is to enable the player to navigate the game world. It should convey the underlying physics simulation and not trick the player into believing the game has more to offer than it actually has. Static props are avoided for the benefit of dynamic props. If there is a jar on a table, the player can interact with the jar by picking it up or simply by shoving it off the table and seeing it break against the floor.

A big field of view is necessary to comfortably navigate confined spaces and interact with the nearby environment without resorting to a third person perspective. The common pin-hole camera model is not suitable for high field of views since it yields big distortions of areas and angles when pushed towards its limits. A trade-off is to use a fisheye perspective that sacrifices linearity for less area and angle distortions. This enables a high field of view with less overall distortion. The recent development of head mounted displays offers an even better alternative by avoiding the mapping of a high field of view of the virtual world on smaller field of view in the real world.


## Sound
Similar to graphics, the audio is there for the player to experience the dynamic game world. Lots of dynamic sounds are emitted by the game world in order to communicate with the player. If two props collide with each other, sound is emitted. If something travels trough the air with enough speed, sound is emitted. If something slides against something, sound is emitted. The breath of non-player characters can be heard just like the breath of the players character. Foot steps, friction, wind, water. Most wherever energy is transferred, there are losses, some of these losses turns into sound. This gives the player a richer perception of the game world and is helpful for navigation and interaction.


## Input and controls
The interaction with the game world is the very core of the game. This puts a lot of demands on the input methods.
