<p align="center">
    <img width="192" height="192" src="https://github.com/romanus-severus/DistanceMatching/raw/main/DM_Plugin.png">
</p>

# Distance Matching
 
This plug-in is custom implementation of the [Distance Matching technique](https://www.youtube.com/watch?v=YlKA22Hzerk) which was shown by Laurent Delayen at Nucl.ai 2016.
In two words, Distance Matching plays the animation frame which corresponds to the distance to marker location (start, stop, pivot, etc.).
Target engine version for this plug-in is UE5, for UE4 see 4.27 branch.

### Features:
- Predicting the stop, pivot, jump apex and landing location.
- Calculating the distance and time to marker location in each frame.
- Custom animation node for playing the animation by the distance.
- Animation Modifier for extracting distance from the root motion animation.

### Restrictions:
- `Uniform Indexable` type of the curve compression should be applied to animation which will be used with DistanceMatching animation node.
- Animation Modifier works only with root motion animations (root motion data is only needed for extracting the distance, you can disable root motion in animation itself).
- `Use Separate Braking Friction` should be disabled (when it's enabled, air resistance will be applied, it will complicate the jump apex and landing prediction).

### Installation:
> For Blueprint project you should add any C++ class.

1. Close Unreal Engine editor.
2. Copy DistanceMatching directory to the Plugins folder at the root of the project.
3. Regenerate the Visual Studio project files.
4. Open Visual Studio solution file and compile it.
5. Open your project and enable the DistanceMatching plugin.

[How to use Distance Matching plugin](https://www.youtube.com/watch?v=47ckE8c28-0)

### How To Contribute:
- Create a personal fork of the project on Github.
- Clone the fork on your local machine. Your remote repo on Github is called `origin`.
- Add the original repository as a remote called `upstream`.
- If you created your fork a while ago be sure to pull upstream changes into your local repository.
- Create a new branch to work on. Branch from `develop`.
- Implement/fix your feature, comment your code.
- Follow the [Epic Games code style](https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/DevelopmentSetup/CodingStandard/) of the project, including indentation.
- Run `FormatSourceFiles.ps1` and commit changes if there is one. Don't forget to [sets the PowerShell execution policies](https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.security/set-executionpolicy) to `RemoteSigned`.
- Squash your commits into a single commit with git's [interactive rebase](https://docs.github.com/en/get-started/using-git/about-git-rebase). Create a new branch if necessary.
- Push your branch to your fork on Github, the remote `origin`.
- From your fork open a pull request in the correct branch. Target the project's `develop` branch.

### Special Thanks:
- [Epic Games](https://www.epicgames.com/)
- [Laurent Delayen](https://twitter.com/LDelayen)
- [Martin Wilson](https://twitter.com/__Arcadia)
- [Nicolas Lebedenco](https://github.com/nlebedenco) 
- [Caleb Longmire](https://twitter.com/clongmire42)
- [Soumik Bhattacherjee](https://www.youtube.com/channel/UCr0oJDvyuqjZDB9rNdhlUNA)
- [Giuseppe Portelli](https://twitter.com/gportelli)