Hello!

This project is a tutorial project for how to create shaders in UE4 (4.7). Most material effects can be created in-editor using the excellent tools that Epic has provided us with.
There are some times though, where you simply want to use a pixel or compute shader to do some work but you don't want this work to end up in a material surface or post process material, but simply have access to
it in a render target, a Cpp texture obect or a struct. This is where basic HLSL shaders really shine. 
And while it is possible to use normal HLSL shaders in UE4, there doesn't seem to be any tutorials for it yet, and there are some complications that make it necessary to take a few detours before you can actually
create and run your shader.
It is worth to note that this is not a tutorial on how to program shaders in general, or how to write HLSL, but rather how to get shaders working in UE4. For learning how to program shaders or HLSL, I recommend other resources,
such as MSDN: https://msdn.microsoft.com/en-us/library/bb509561(v=VS.85).aspx or instructional books for more advanced users such as "GPU Pro" by Wolfgang Engel.

The HLSL:
There is no simply way to compile a shader and have it work properly like you can in a simple project. I haven't investigated it much, but compiling your shader from an inline string or loading a file and compiling your shader
code does not seem like it is an easy task in UE4. Instead the recommended way is to place your shader code in so called *.usf files under your engine's Engine/Shaders/ directory. Depending on if you are using a binary or source
build of the engine, this will obviously change which Engine/Shaders/ folder you actually have to use.

The shader compilation:
When you have placed your shader code in the correct folder, you need to compile it. This is done using a macro in your Cpp source called IMPLEMENT_SHADER_TYPE.
One problem with this though is that if you simply place this and your other code in your main project, you are going to have problems since this macro must be available and run during an early phase of engine start up, 
and in addition to this cannot be rerun if you decide to recompile your code in-editor. Ignoring this will actually crash the engine :). The solution is to put the shader declaration and initialization into a plugin and override
the load phase of the plugin in question to "PostConfigInit".

Shader parameters:
Shaders take their input from the generic ParameterMap system in UE4. You can program this from your shader declarations using a combination of macros and code calls. The macros are generally used to declare structures to be
used in the shaders, such as uniform / constant buffers etc. And the code parts are to bind runtime instances of these buffers to the ParameterMap. You can also bind resources like textures to the shaders if you want to.

Shader invocation:
When you have declared your shader in the appropriate plugin, you can now start up your project and start using it. Shaders in UE4 have to be invoked on the rendering thread, to hopefully nobody's surprise. To make sure 
we get our code to run on this thread, Epic has given us another handy macro called ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER. There are variations of this macro that let you send more than one parameter too, but since
we only want to send ourselves, one parameter does fine. We simply instruct the render thread to take a pointer to our shader consumer object and call a function on it. It is inside this function that we can do all the
appropriate calls to set rendering states, buffers, shaders etc. as well as dispatch rendering calls. All rendering functions are accessible through an abstraction layer called the RHI "Rendering Hardware Interface",
which as the name suggests is a way for our code to be platform independent. Most function names on the RHI seem to be inspired by directx rather than any other API though, which might be good to know. If you are
having trouble finding a particular function, the best way is therefore to check MSDN before any OpenGL resources or otherwise.

Shader output:
After you have run your shader it is of course time to harvest your output. There is no special UE4 magic to this step as it uses the normal SetRenderTarget RHI functions for pixel shader output and your UAVs for
compute shader output.


How to run this project:
To get the project to run, you first need to bind it to an engine, you do this like any other project by right clicking the uproject file and switching version if needed, and then doing Generate Project.
You then need to open up the solution file and build the source code.

How to use this project:
I would recommend checking out the example project supplied in this repository to understand how to best use the project code. All the relevant files in the repository are:
* Everything under the Plugins/ folder                     (For declaring the shaders)
* Everything under the CopyToEngineShaders/ folder         (For copying into the Engine/Shaders/ folder, these are the actual HLSL shaders)
* Source/ShaderPluginDemo/ShaderPluginDemoCharacter.cpp/.h (These are changes and additions to the main character class in the demo project to allow for the shader's use. I have removed all non-related comments in these files to make it easy to see where I have made changes)
* Everything under the Content/ShaderPluginDemo/ folder    (These are the editor objects that I use to set up the shader use in the scene)
* The project settings file                                (I have created some new input bindings)

Project controls:
W/A/S/D - Movement
Space - Jump
Move mouse - Look around
Left mouse button - Paint the object you are aiming at with the output of the compute shader/pixel shader chain
Q/E - Change the blend amount on the pixel shader. Q moves the blend closer to the pixel shader simple gradient, while E moves the blend closer to the compute shader output.

General comments on the demo project:
The demo project demonstrates how to use both pixel and compute shaders using the method I have explained. In the tick function, I first run a compute shader that generates a cool visualization of a galaxy entirely within the shader.
This is then packed into a R32_UINT format to allow for usage within the pixel shader which is then run that simply generates a gradient and then blends with the compute shader output. 
The user can shoot objects in the demo scene to apply a dynamic material instance to the object hit that is then loaded with the rendertarget containing the pixel shader output.
You can also modify the amount of blending that the pixel shader does by holding the Q and E buttons.

I hope someone finds this useful :)

Best regards,
Temaran







Troubleshooting:

Some people have had problems with the shadercopyhelper and are getting shader related errors such as:
Assertion failed: ShaderRef != NULL && *ShaderRef != NULL [File:Runtime/ShaderCore/Public/Shader.h] [Line: 926]
Failed to find shader type FComputeShaderDeclaration

Basically, the plugin defines its shaders in *.usf files. These *.usf files can only be read by the engine if they reside in the Engine/Shaders folder.

This is of course a problem, since you cannot define these on the project level, and if you want to leverage the system, you have to change your engine setup.

The way I tried to solve this problem with the plugin was to create a different plugin that basically just copies the shader files to the correct location early in the editor launch process.

This seems to work for most people, but sometimes, it fails for some setups it seems.



I can think of a few things you can try to fix this:
*I've only tested this tutorial with 4.7, so if you happen to run a different engine version, it might not work because of that.

*Ensure that the Project/Shaders folder actually contains the *.usf files and those have not been corrupted or deleted or something silly like that.

*If the files are there, you can try to remove the ShaderCopyHelper plugin (under plugins) and manually copy the usf files to your Engine/Shaders folder.
