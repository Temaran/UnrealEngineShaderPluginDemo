**Notice**

I have removed the master branch and updated default to be the latest major version instead to make it more clear what major version head belongs to. I'm currently working on an update for 4.24 since there has been some major changes in how you best work with global shaders in the engine.

**Basics**

Updated version of Temaran's Compute Shader Plugin to Unreal **4.21**. For more details see Temaran's original project: https://github.com/Temaran/UE4ShaderPluginDemo

**Details**
* In UE4.21 Epic removed the automatic inclusion of virtual path to Shaders in the plugin directory. The project was updated to explicitly add the virtual shader paths.
* Modified ComputeShader so it works on macOS as well, the shader function imageSize() is not available in Metal, or is broken in Epic's shaders.
* In UE4.20, Epic changed the way the rendering pipeline is accessed and added global shaders for plugin (for details see the 4.17 release notes). This has made the "ShaderCopyPlugin" superfluous.
* On the first start, the shaders will get compiled by Unreal. This might take some time, so be patient. ;)
* This is a project plugin atm, as designed by Temaran. Unfortunately, I wasn't able to convert this into a engine plugin so far. When I manage to do this, I will insert it here in a new branch. If someone can/wants to help however, this would be cool as well!
* For problems and other feedback contact me via my portfolio: www.valentinkraft.de :)
