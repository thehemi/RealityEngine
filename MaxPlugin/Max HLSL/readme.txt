Tim: Outdated readme that came with the inital release I converted


Updated Features
----------------

This version of the plugin addresses several problems with the previous release, and adds useful functionality.  Most noticeable are:

* File Save/Load *

An updated file save/restore method for the plugin.  This is designed to make the plugin future-proof as the .fx file format evolves and to address performance problems when reloading large files. The plugin still supports the old format, but it will advise you to update all old files as soon as possible, simply by re-saving the MAX scene.  Once scene files are updated, reload performance should improve by about 4x.  

It is possible that if you created a HLSL file on an early version of the HLSL runtime, the plugin will have difficulty reloading shader parameters due to fixes since the HLSL Alpha program.  The plugin should still manage to reload the file, assuming an updated version of the .fx file is present, but some parameter mappings may be lost.  The plugin will tell you to update these.  Due to the new file format, this should not happen in future (plugin parameters and all their animation keys are now stored with the material seperately from the MAX parameters).  Note that if for any reason the plugin fails to load/apply a .fx file, it will revert to default.fx.

* Miscellaneous Bug fixes *

Many bugs have been fixed in this release, among them:
- Problems with polygon normals across multiple smoothing groups
- Severe performance problems adding/removing/moving lights on complex scenes.
- Performance problems generally manipulating objects.
- Problems mapping effect parameters to new effects when swapping effect files.
- Applying texture coordinate mappings did not always immediately update the object in the scene.
- Texture preview items in the connection manager could cause a crash on textures with 24 bit formats.
- L8 textures were not correctly supported.


* Scripting support *

The plugin can now be scripted.  In the example directory is a script called cg_regression.ms, which when run will request a pointer to a cgfx file directory, create a selection of spheres using the .fx files in the directory, and apply them to a spiral.  On completion, a log file will appear listing each loaded .fx file, and the value of each parameter in the effect.
Note that the 'undo off' wrapper around the viewport manager section of the script is essential due to problems with the 3DStudio MAX viewport manager.

* An about box * 

This gives details of all the Cg/CgfX plugin components being used, as well as information on any versions of components not compatible with the plugin.  Please note down the numbers when reporting any problems to NVIDIA.





