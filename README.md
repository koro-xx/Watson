#Watson, a puzzle game

Watson is a clone of “Sherlock”, an old game by Everett Kaser which is itself based on the a classic puzzle known as ["Zebra puzzle"](https://en.wikipedia.org/wiki/Zebra_Puzzle) or “Einstein's riddle”.
   
<p align="center">
<img src="https://github.com/koro-xx/Watson/blob/master/screenshots/watson-screenshot-1.png" width="400" />  <img src="https://github.com/koro-xx/Watson/blob/master/screenshots/watson-screenshot-2.png" width="400"/>
</p>

##How to play

The board is partitioned into an n x h grid of blocks of same-type items (letters, symbols, greek letters, etc). The goal is to figure out which item goes in each block. Each item should appear exactly once. The game provides clues in two panes (right pane has horizontal clues, bottom pane has vertical clues. Each clue tells you something about the relative position of items in the solution. Depending on the configuration of the main panel, a clue may help you discard some item from a given block. For instance, if a vertical clue tells you that the column of the 'F' letter is between the columns of the symbol '?' and the number '3', this tells you that 'F' can't be in the first or in the last column. If another (vertical) clue tells you that the letter 'F' is in the same column as the greek letter alpha, then you can rule out alpha from the same columns where you ruled out 'F'.

For an explanation of the meaning of each clue, left-click the clue. To get used to the game, it may help to ask for a few hints, which will show you this kind of reasoning. This is done by clicking on the '?' button on the bottom-right corner. 
This will also tell you if you made a mistake (for instance ruling out an item that could not be ruled out).

The default board size is 6 x 6, but you can change it in the settings to any size from 4x4 to 8x8 (different width/height is also possible). The clues provided are guaranteed to lead to a unique solution. There is an 'advanced' option that generates much more difficult puzzles. These assume more indirect reasoning (like assuming that an item is in a given block and seeing what happens, then ruling it out if it leads to a contradiction). In my experience, advanced games tend to be almost impossible for a 6x6 or higher size board (I hope to tune this later). Also, advanced games won't always provide hints. I recommend against using this setting until it is improved.

#Assets

Watson is programmed in plain C and uses the Allegro 5 library. Big thanks to the friendly folks from #allegro in Freenode for all the tips and advice.

The GUI uses SiegeLord's [WidgetZ library](https://github.com/SiegeLord/WidgetZ) (included with the sources).

The main text font is [Linux Libertine](http://linuxlibertine.sf.net/) by Philipp H. Poll, licensed under the GPL. The TTF tiles used are a combination of symbols from different fonts downloaded from www.fontlibrary.org.
The sounds are from www.freesound.org. Button icons and bitmap tiles are from www.icons8.com, licensed under [Creative Commons CC BY-ND 3.0](https://creativecommons.org/licenses/by-nd/3.0/).

The code itself is licensed under the GPLv3 (this excludes the above items).

Note: The tile set is rendered from TTF fonts, but there is also an option to load custom bitmap tiles in the settings. If you want to change the tiles, they should be stored in <APPDIR>/icons into 8 separate folders named 0 to 7, each with 8 square tiles name 0.png to 7.png. 

##Build instructions:

You need Allegro 5 (>= 5.1.9) and cmake.

For Mac OS X (tested with Xcode 5):

	cd watson && mkdir build && cd build
	cmake -G Xcode ..
	xcodebuild -config Release

The app bundle will be in the "Release" folder.

For windows (tested in Visual Studio 2015)

	cd watson
	mkdir build
	cd build
	cmake -G "Visual Studio 14 2015" -DAlleg_ROOT="\path\to\allegro\libs" 
	path\to\msbuild.exe watson.vcxproj /p:Configuration=Release

or open the project in Visual Studio and compile for release. After that we need to copy the resources found in the "watson" folder (fonts, icons and sounds) to the application folder, together with all the required dll's (that is, allegro dll's + runtime). 

For Linux: 

	It should work similarly, just remember to copy the /fonts and /sounds folders to the app dir.

For Android: The game now works on Android devices as well. Compile instructions will come later.

## TODO

- Tune the difficulty of 'advanced' mode. 
- Add extra settings to configure the number of revealed blocks and tune other aspects of puzzle generation.
- Fix CMakeLists.txt to include windows icon.
