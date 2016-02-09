#Watson, a puzzle game

Watson is a clone of “Sherlock”, an old game by Evertt Kaser which is itself based on the a classic puzzle known as ["Zebra puzzle"](https://en.wikipedia.org/wiki/Zebra_Puzzle) or “Einstein's riddle”.

Watson is programmed in plain C with the Allegro 5 library. Big thanks to the friendly folks from #allegro in Freenode for all the tips and advice.
   
The tile set is rendered from TTF fonts obtained from public repositories (fontlibrary.org, freesound.org). There is also an option to load custom tiles, which should be stored in APPDIR/icons into 8 separate folders, each with 8 square tiles in .png format. These won't look as nice as the fonts due to the anti-aliasing.
  
There is an "advanced" mode that should generate harder puzzles, but I don't know how well it works (or if they really are harder; more testing is needed).

##Build instructions:

You need Allegro 5 (>= 5.1.9) and cmake.

For Mac OS X (tested with Xcode 5):

	cd watson
	mkdir build
	cd build
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

