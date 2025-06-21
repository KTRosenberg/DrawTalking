# Make The Thing - DrawTalking
- A prototype for building interactive worlds and telling stories at the same time by sketching and speaking
- [**UIST 2024 research publication**: "DrawTalking: Building Interactive Worlds by Sketching and Speaking"](https://dl.acm.org/doi/10.1145/3654777.3676334)
- (Also a main component of my Ph.D. thesis.)
- **Website**: [https://ktrosenberg.github.io/drawtalking/](https://ktrosenberg.github.io/drawtalking/)
- **Outreach**: please feel free to reach-out! (Use the e-mail on the website) I'm open to collaborations and conversations.
- **Questions**: as part of outreach, I'm happy to answer questions. The best starter documentation is on the web-page.



https://github.com/user-attachments/assets/e43605af-f2cd-40a7-b78a-08faa7dc4354



## Contents
- [Materials](#materials)
- [Referencing This Work](#referencing-this-work)
- [Project Overview](#project-overview)
- [How It Works](#how-it-works)
- [First-Time Set-up](#first-time-set-up)
- [Running](#running)
- [Extending System Capabilities](#extending-system-capabilities)
- [Project Files of Interest](#project-files-of-interest)
- [Miscellaneous](#miscellaneous)

<div align="center">
<img src="/resources/DrawTalking_UIST_poster.png" alt="Poster Lower-Res" style="width:47%; height:auto;">
</div> 
  
## Materials
- [30-second Preview Demo - Some Sped-up](https://youtu.be/Kgt48theqi0?si=FSAM6PkCeO7XPo5G)
- [Demo Reel - Some Sped-up](https://youtu.be/2i1ZeNQdjBQ?si=kCiH3ARdFOq-Fogp)
- [Full Real-time Demo Videos Playlist](https://youtube.com/playlist?list=PLlSARs4-8fLXeRAIqbdXXlXAeniFnq3bz&si=ug7s3K6UH1J6Otkq)
- [Paper (self-hosted mirror)](https://ktrosenberg.github.io/drawtalking/DrawTalking.pdf)
- [ACM Digital Library Page (Open Access to Anyone)](https://dl.acm.org/doi/10.1145/3654777.3676334)
- [UIST Poster](https://ktrosenberg.github.io/drawtalking/DrawTalking_UIST_poster.png)
- [(Starter) Instructions 2023](https://dl.acm.org/doi/suppl/10.1145/3654777.3676334/suppl_file/DrawTalking_Info_Sheet_2023.pdf)
  - This is not exhaustive, but rather is just a subset given to user study participants. There are many more actions possible. The paper design sections go more into detail. Documentation will be updated over time.
 
## Referencing This Work
- Please give credit when referencing or using the project. Here is the BibTeX for citing the work:

```
@inproceedings{10.1145/3654777.3676334,
	author = {Rosenberg, Karl Toby and Kazi, Rubaiat Habib and Wei, Li-Yi and Xia, Haijun and Perlin, Ken},
	title = {DrawTalking: Building Interactive Worlds by Sketching and Speaking},
	year = {2024},
	isbn = {9798400706288},
	publisher = {Association for Computing Machinery},
	address = {New York, NY, USA},
	url = {https://doi.org/10.1145/3654777.3676334},
	doi = {10.1145/3654777.3676334},
	abstract = {We introduce DrawTalking, an approach to building and controlling interactive worlds by sketching and speaking while telling stories. It emphasizes user control and flexibility, and gives programming-like capability without requiring code. An early open-ended study with our prototype shows that the mechanics resonate and are applicable to many creative-exploratory use cases, with the potential to inspire and inform research in future natural interfaces for creative exploration and authoring.},
	booktitle = {Proceedings of the 37th Annual ACM Symposium on User Interface Software and Technology},
	articleno = {76},
	numpages = {25},
	keywords = {creativity, human-AI collaboration, multimodal, play, programmability, prototyping, sketching},
	location = {Pittsburgh, PA, USA},
	series = {UIST '24}
}
```

## Project Overview
- DrawTalking is a result of of  rapid user experience design and interactions prototyping within my personal playground, which is called "Make The Thing" or "mtt" for short. (It's meant to be a motivating name to get things done.) The purpose of DrawTalking was to realize a prototype for a new way of interacting with and building worlds. Drawing and talking together let you do two things at the same time: 1) interact with other people the same way you would as when giving a whiteboard talk or brainstorming, or when telling stories to others 2) control many interactive elements in the system at the same time, using roughly the same drawing and speech we'd do to communicate as normal. A perfect version of this concept might give people the ability to communicate with others as naturally as usual, but with the additional capabilities of computation and graphics integrated into how we normally behave. This project was one step towards this idea.
- I'm the sole programmer for this project and can speak to all of its functionality: system design, UI & UX, graphics rendering, etc. Some open-source libraries support the project. There are many additional features not discussed in the paper such as a built-in command line prompt, image loading, code reloading on desktop, initial camera device support, and others. DrawTalking only uses some of these features. I also used mtt as an opportunity to learn new technologies such as Apple's lower-level graphics API, Metal, multimodal input/multitouch programming, UI-design, and so on. The DrawTalking application came about through many steps including formative user studies and observations, storyboarding and concept art to get things visualized, several versions of the UI/UX, my Ph.D. thesis, an ACM CHI Late-Breaking Work (work-in-progress publication), and finally the UIST '24 publication above. Thanks for my mentors and advisor for their support!
- In short, this is an initial release of an ongoing personal project. The code represents a lot of research and rapid prototyping.
- This repository is only a snapshot of what I made possible for the thesis and UIST publication. I wrote the project to be fairly easy to expand. For example, the set of functionality and behaviors that are included can be extended and combined with other software. My hope is that by releasing the code, others (researchers and creativies especially) might have a baseline reference for how a system like this and all of its components were pieced together, as well as something to play with and experiment with in tandem with their own projects. DrawTalking is not meant as just a standalone application, but rather as a potential backend for other software that might use its functionality or take inspiration.
- Please check the website for the paper and demo videos.

## How It Works
- The paper (the design section especially) is the manual. It describes in-detail how the interface works. Here are a few of the main interactions to get started, but in general, please refer to the paper.
1. Create drawings (at any time) with the stylus. After starting to draw a new object, all subsequent strokes will be added to the drawing. Tap the background to de-select the drawing. Select the object again to continue adding to that drawing. Otherwise, a new drawing will be created. If drawings are overlapping at a point, if you select at that point, the system will just select the smaller sketch. Standard multi-touch pinch/zoom works in DrawTalking.
2. Tap them in the order in which you'd like to name them. "This is a \<object name e.g. dog> and that is a \<other object name e.g. ball>" You can select several at once and give them all the same name.
3. To do a speech command, say something like "The dog moves to the ball." Then tap the language action button on the top right of the screen (or squeeze the Apple Pencil if you have an M4 iPad and the Apple Pencil Pro). This will stage the command. Confirm the command by activating the same button again. Optionally beforehand you can see the diagram on the top-left, which shows the objects selected for the command. (This lets you make sure everything's correct before confirming. (Watch the full demo videos to see how quick it's meant to be done. In most cases, it'll be correct for reasonable input and when there's no ambiguity around which objects should be selected.)
3.2 Optionally, keyboard input works too. Typing will show a command-line interface at the top of the screen. Type `in <your command>` to enter the equivalent of voice input and then `shift + return` to do the equivalent of activating the confirmation button. There are other commands remaining from personal tests, including texture loading, the Augmented Reality camera, and other development features. You can find those in [make_the_thing.cpp](</apple_platforms/Make The Thing/make_the_thing.cpp>).

## First-Time Set-up

### Prerequisites
- The main set-up is an iPad client with a mac serverside. The intended user experience configuration is an iPad Pro (M2 or M4) connected with a mac computer with an M1 chip or later. A macOS-only version is also available.

#### Hardware Requirements for Server-Side
- Apple computer with M1+ chip running macOS 15.5
- Getting a version of the server running on other operating systems may be possible, but for now, this is untested.

#### Hardware Requirements for Client-Side
- iPad version: Apple iPad M1 or later and Apple Pencil running iPadOS 18.4 or later; intended experience tested for iPad Pro M2, M4 (Other M non-pro models should work, but with different UI layouts that the one used for the demo. M1 should work, but with reduced hover support and no squeeze support. The squeeze on the pencil pro for M4 activates a speech command so you don't have to press the button, but otherwise an M2 or M1 is fine.)
- macOS version: Apple computer with M1+ chip running macOS 15.4.1+.

#### Preparing to Build Client Application
### Xcode Project for Client Application (steps on macOS Desktop)
- enable both Siri and dictation in system settings. (DrawTalking uses the strictly on-device speech recognition.) As of more recent OS updates on iPadOS, enabling dictation has often been required to trigger system downloads in the background to support on-device recognition. On macOS, Siri might be enough, but temporarily enabling dictation is recommended if this doesn't work. This might take a few minutes.
- download Xcode (version 16.4, or the version with an SDK matching your OS.) and command line tools from the Apple developer website [https://developer.apple.com/xcode/](https://developer.apple.com/xcode/)
- make sure you're logged-in with an Apple ID in Xcode / set-up building projects with your Apple ID (Xcode > settings > accounts, add an account if you haven't). You do not need to be enrolled in the Apple developer program to use your Apple ID.
- from the project root, find and open the Xcode project at [/apple_platforms/Make The Thing/Make The Thing.xcodeproj](/apple_platforms/Make The Thing/Make The Thing.xcodeproj).
- in Xcode, go to the signing page in the project for both macOS and iPadOS targets by clicking `Make The Thing` in the left menu (at the top), and then selecting `Make The Thing` in the TARGETS list. In the top horizontal menu, pick `signing and capabilities`. Switch your Team, and change bundle name to use your own name. (it could be a free non-developer account available with an Apple ID, not just the developer program one).
- Note: if you try to build with the play button without setting-up signing and teams, there will be an error with a red icon or a message. Click these to be taken to the correct page.
- Set your iPad to developer mode to support building to it: 
- optional: pick the target device (iPad or mac) at the top of the window. There should be a "Make The Thing" button, which means the iPadOS version is selected. Click the button and switch it to the macOS option in the drop-down menu if you'd like the developer macOS standalone version (not recommended since the best experience is with the iPad version). Select your device by using the button beside the "Make The Thing" button to open a drop-down menu. You should plug an iPad-in with a thunderbolt cable so Xcode can copy the project to your iPad. Follow these directions: [https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)
- optional: the project should already be set to build at the best performance mode (release mode) with no debugging. To check or change this, look at the top, pick the iPadOS or macOS target (or for both), and edit the scheme by clicking the Make The Thing button and selecting "edit scheme." The build configuration option should show the release mode with the debug checkmark disabled. For quicker building and debugging, but non-ideal performance, switch the option to debug mode and check the debug checkmark.
- the project should build; no speech recognition or language features will work yet without the server component.
  
### Installation for Local Server Running on macOS Desktop
- install the exact Python version `3.9.13` (that can co-exist with other Python installations if you have any): scroll down this page and click the universal2 installer: [https://www.python.org/downloads/release/python-3913/](https://www.python.org/downloads/release/python-3913/)
- install NodeJS [https://nodejs.org/en/download](https://nodejs.org/en/download) - the package installer for the latest LTS version is recommended.
- open the terminal
  - install the Rust toolchain and make sure it's on the path (follow the directions in the terminal)
      - [https://www.rust-lang.org/tools/install](https://www.rust-lang.org/tools/install)
      - As of writing, this is the command, but the page above will be up-to-date:
      
      `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh`
      - When the rust installation process is done, close/reopen a new terminal window for the quickest way to proceed with the rust toolchain enabled. Make sure the command `rustc` works.
  - in the terminal, change the directory to the root project directory and down to the `serverside_common` directory
  - download `coreferee-container` from [https://drive.google.com/file/d/1RvvZFpmfc7EoUzhnlv8-BygMx4nOG3Nw/view?usp=sharing](https://drive.google.com/file/d/1RvvZFpmfc7EoUzhnlv8-BygMx4nOG3Nw/view?usp=sharing) and unzip it into `serverside_common` (This is a minimal preserved clone of the [coreferee repository](https://github.com/richardpaulhudson/coreferee?tab=readme-ov-file#version-120). Version 1.2.0 and no later are required.)
  - run the main installation script with the command (This installs all Python dependencies in isolation using a virual environment.):
    `. ./init-macos-arm64.zh`
    - NOTE: the initial dot before specifying the script is important to make sure the script runs in the correct environment. 

## Running
- start the server on your desktop by starting at the project root directory in the terminal and changing to the sub-directory: `serverside_common`.
- run the server with

  `./run-macos-arm64.zh`
- Start-up takes a short amount of time. The server (called "Lingua") prints `Loaded model successfully` to the screen after a series of `initializing Lingua ...` print-outs. When start-up is done, you'll see a printout for "Ready!" At this point, the server is ready to connect. The next steps refer to building or starting the client application.
- in Xcode, build the client application for the desired target (iPad or macOS), or if it's been built already, you can open the application on your iPad or mac without Xcode by searching for `Make The Thing` in Spotlight. (For free accounts as opposed to Apple developer program accounts, you may need to rebuild every so often.). The iPad should be connected to the mac ideally with a thunderbolt cable when building the first time. Afterwards, the application will be on the iPad.
- the very first time building may ask for permission to use features such as speech recognition. Accept these. On macos, you'll be asked to set the data directory. The default Documents folder is fine.
- Next if building the application for the iPadOS target:
  - Connect your iPad and mac to the exact same WIFI network as the mac running the server
  - Launch the application on the ipad (automatic if building). you'll see a form to fill information for the host address and the port. The host field should be set to the IP Address of the mac machine running the server. You can find it any way you prefer. For example, using the OS GUI on your mac go to `system settings > network > Wi-Fi`. Click the details button next to your wifi connection. The (shorter) IP Address listed on the page is what you should enter for `host` in the form. This will be saved for subsequent runs, but needs to be updated whenever your network connection changes, or every so often when the IP might change. The `port` field should be set to `8080`.
- Next if building the application for the macOS target:
  - Assuming the mac is running both the application and the server:
    - For the host, enter `localhost`
    - For the port, enter `port`
    - Alternatively, you can press the default configuration or localhost button
  - If the server is running on a machine than the client client, the client/server connection instructions are the same as for the iPad as written above. (You need to enter the host's IP address and port 8080.)
- Enjoy!

## Extending System Capabilities
### Actions / Behaviors
- Arbitrary actions and behaviors can be added to the library. The built-in library is in  [standard_actions.cpp](</apple_platforms/Make The Thing/standard_actions.cpp>). For the moment, the best thing to do is to take a look there.
### Natural Language Processing
- I've included a way for you to add your own natural language preprocessing step before the spacy library in the existing system processes the user input (the speech to text or typed input). The idea is that you can try to support greater flexibility in how you talk by using your own algorithms and techniques to pre-simplify the text into something spacy can reliably parse and DrawTalking can use. You can install your own libraries in their own environment that is isolated from the system's environment. The preprocessed text would be sent to the default processing step in lingua.py. A goal would be to simplify the input as much as possible for the final step to improve the results (e.g. reduce ambiguity, make parses as consistent as possible, capture intent better, and so on)
- Simplifying text into another form of raw text is not necessarily the best way to extend the natural language processing, but it's a straightforward task to try in the near-term without having to rewrite parts of lingua.py. I wanted to provide a short-term entryway to playing with this part of the system. Other things to try in the future might be to output a domain specific language with custom parsing and interpretation into tagged sentences. These could be easier to verify than simply outputting simplified text. It would be interesting to support conversational, free-flowing input and convert that into a DSL with only the relevant parts tagged. There are several creative possibilities to support more natural speech.
- For now, the main system requires the specific installation and library versions as described in previous sections, which is why the current solution is to give you an isolated environment in which you can install and use whatever you'd like without interfering with the system's required libraries. To integrate your own preprocessing, create a virtualenv in the [serverside_common](/serverside_common) directory called `py-ext`. The command might look like `python3.12 -m venv py-ext` (or use any version of Python). Install any libraries specific to this external environment using `source py-ext/bin/activate` and e.g. `pip3 install <...>`s. `deactivate` when done.
- To write your own preprocessing script that outputs a revised version of the input text, edit the [serverside_common/script-language-processing-extern.py](</serverside_common/script-language-processing-extern.py>) file. Check the comments in this file to get started. If you ever need to reset to the original, copy the provided [serverside_common/script-language-processing-extern-default.py](</serverside_common/script-language-processing-extern-default.py>).
- You also need to enable this external preprocessing: the init script creates a configuration file for you to edit called  `serverside/script_extern_config.json`. For reference, a default version of the file with external scripts disabled is included: [script_extern_config-default.json](</serverside_common/script_extern_config-default.json>).
- In `script_extern_config.json`, change the value associated with the `isExternalEnabled` key from `false` to `true`. This causes your preprocessing script to receive the user input first through stdin. Redirection to the rest of the original pipeline is done for you. You just need to modify the input text if you'd like. Otherwise, the behavior is exactly the same and data are just passed through.
- Note: for those interested, the system could be modified further to support any language or technology so long as the correct protocols are followed for reading from stdin from the server and writing to stdout. The exact format needs to be followed when writing from your custom script to stdout so lingua.py can read it correctly. As long as you follow these constraints, you can change the configuration file to start-up and run any program in its own process.
- Warning: the assumption is that messages won't get too large. The project currently uses SpaCy 3.1.4's transformer model, which warns that it has a hard-limited sequence length of 512 tokens at-maximum. In-practice, input shouldn't approach that size since most of the time we'll speak with shorter utterances than something that would be essayic. A more general-pupose version would chunk the data.
- Updating to a newer version of SpaCy would require updates in the DrawTalking application to support changes in tagging and output. It's doable, but out-of-scope at the moment. This is why I'm providing a way to add your own code in-between, so new things can be tried now without needing to change DrawTalking.
  
## Project Files of Interest
- For those interested, the application is split between platform-specific Apple code and more general agnostic C and C++ code that interfaces with the platform-specific code. It's possible to port to other platforms in the future this way that can run regular C/C++ code and implement a different platform-specific backend. The renderer also works this way (Metal in the backend, a generic frontend).
  ### Starting to explore the project
  - The main logic in DrawTalking is handled in [make_the_thing.cpp](</apple_platforms/Make The Thing/make_the_thing.cpp>) - user input, the world, per-frame animation, updating rendering, interacting with the drawtalking natural language processing and running the behaviors. It's the root where almost everything is called per-frame.
  ### Thing
  - The subsystem that handles "things" in the world and the interactive scripts/logic that have them work with each other: [thing.hpp](</apple_platforms/Make The Thing/Make The Thing/thing.hpp>) and [thing.cpp](</apple_platforms/Make The Thing/Make The Thing/thing.cpp>). Pieces of this include a mixed message passing + execution graph system.
  ### DrawTalking
  - Most files with a "drawtalk" prefix handle the interpretation of natural language into runtime scripts or involve many UI/UX components that use this information. Check [drawtalk.hpp](</apple_platforms/Make The Thing/Make The Thing/drawtalk.hpp>) and [drawtalk.cpp](</apple_platforms/Make The Thing/Make The Thing/drawtalk.cpp>) to start with the main pieces for compiling natural language into runtime scripts. [drawtalk_ui.hpp](</apple_platforms/Make The Thing/Make The Thing/drawtalk_ui.hpp>) and [drawtalk_ui.cpp](</apple_platforms/Make The Thing/Make The Thing/drawtalk_ui.cpp>) mainly relate to the textual visualization of language within the user interface.
  - The files where most of the built-in actions / behaviors that can be recognized are: [standard_actions.hpp](</apple_platforms/Make The Thing/standard_actions.hpp>) and [standard_actions.cpp](</apple_platforms/Make The Thing/standard_actions.cpp>).
  - There's potential to extend this library with completely different or arbitrary behaviors and functionality. The behaviors are meant to be fairly modular.
  ### Rendering
  - The front-end (and wrapper for platform-specific code) for the rendering system (mostly for 2.5d graphics): [stratadraw.h](</apple_platforms/Make The Thing/Make The Thing/stratadraw.h>), [stratadraw_platform_apple.hpp](</apple_platforms/Make The Thing/Make The Thing/stratadraw_platform_apple.hpp>), [stratadraw_platform_apple.mm](</apple_platforms/Make The Thing/Make The Thing/stratadraw_platform_apple.mm>). It works by having the program specify different layers (strata) of renderable objects that can be rendered in different orders and updated via handles. The program specifies render commands via a command queue. In effect, different groups of object rendering can be turned on/off in the program logic pretty easily. The render supports drawing arbitrary shapes and curves, loading images and textures, some compute shaders, instancing, and multiple render passes and targets, among other features I've experimented with. Not every feature was used in DrawTalking, but the current implementation is versatile-enough to play with and build-on/improve for similar projects.
  - The actual platform-specific graphics (Metal in this case on the Apple platforms) is in [sd_metal_renderer.h](</apple_platforms/Make The Thing/Make The Thing/sd_metal_renderer.h>) and [sd_metal_renderer.mm](</apple_platforms/Make The Thing/Make The Thing/sd_metal_renderer.mm>). The main frame-loop called per-frame is `- (void)update:(nonnull MTKView*)view` in [sd_metal_renderer.mm](</apple_platforms/Make The Thing/Make The Thing/sd_metal_renderer.mm>). The main [make_the_thing.cpp](</apple_platforms/Make The Thing/make_the_thing.cpp>) program code is called within `update` (See `MTT_on_frame(self->_core_platform)`, which is in [make_the_thing_main.mm](</apple_platforms/Make The Thing/Make The Thing/make_the_thing_main.mm>), which calls `mtt::on_frame(&core_platform->core)` in [make_the_thing.cpp](</apple_platforms/Make The Thing/make_the_thing.cpp>)).
  - I achieved text rendering by heavily-modifying [an outside Metal port of the nanovg vector graphics library](https://github.com/ollix/MetalNanoVG) to work with my renderer. (It would be useful to move towards something more maintainable in the future, but the library is very flexible and small, and was useful for this project.)
  - Note: Curve / line rendering: since curves are geometry as opposed to immediately-rasterized pixels on a texture, more and more data are created as the user draws. To avoid discarding the data as the line gets longer (during pen move), the line continues from where it left-off in a staging buffer. This avoids slowing-down the system because it doesn't have to recompute the entire curve every frame; just the new part added in the latest frame. Note: the lines look best on iPad since there it supports variable-width thicknesses using he Apple pencil 2 or pro. The rendering of the lines themselves could be rewritten and improved in the future though, as they start losing fidelity at high zoom levels. Curve simplifications at small values could also help. GPU-driven / compute-based creation and rendering of the curves might be yet another option.    
  ### Platform Set-up
  - The most relevant platform setup launcher code where the actual application starts is in [MTTViewController.mm](</apple_platforms/Make The Thing/Make The Thing/MTTViewController.mm>) and [MTTViewController_Swift.swift](</apple_platforms/Make The Thing/Make The Thing/MTTViewController.mm>) (they are bridged).
  - Serverside: check the [serverside_common](/serverside_common) folder for the [serverside_common.js](serverside_common/serverside.js) server, which communicates with a Python process [lingua.py](serverside_common/lingua.py) that handles the natural language processing. 
  ### Live Code Reloading
  - The macOS version has access to the entirety of the OS's features, including compiling and linking code at run-time. It's a great fast-iteration development feature! In the macOS version. This is only in debug mode and should generally not be used outside your own local development environment. Set the build target to debug mode and build. As the application runs, you can edit [dynamic_main.cpp](</apple_platforms/Make The Thing/dynamic_main.cpp>) in an external editor, save, then go back to the application to see the changes reflected. (This is raw C++ code, so use at your discretion. Changes in data-layout will likely break things.) Try setting the `ENABLE_ALL_TESTS` macro near the top of the file to 1 instead of 0 to see a test related to simple 2D color blending, for example. This file is called from [make_the_thing.cpp](</apple_platforms/Make The Thing/make_the_thing.cpp>).
## Miscellaneous
- I've included the open-source 2D physics library Box2D (version 3.x). The project didn't use it, but the library could help open-up more types of physics-based interactions out-of-the-box.
