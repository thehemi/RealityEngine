-- Create a sphere
mysphere = sphere radius:28.3637 segs:38 mapCoords:on isSelected:on

-- Create a new standard material
newmat = StandardMaterial() 

-- Assign it to the sphere
$.material = newmat

-- put a light near the sphere.
mylight = Omnilight rgb:(color 255 255 255)
move mylight [-45,-45,0]

manager = null

undo off
(
-- Get the viewport manager from the material 
manager = DXShaderManager.getViewportManager newmat 

-- If there is no manager, assign a new one 
if (manager==null) then manager = DXShaderManager.addViewportManager newmat 
)


effect = manager.setviewporteffect 1
manager.activateEffect newmat true

-- for i = 1 to num do 
-- (  
-- 	if(manager.getViewportEffectName(i) == "Cg") then effect = manager.setviewporteffect i 
-- ) 
--if(effect == null) then messagebox "Could not find Cg plugin"

effect.cgfx = "aniso.fx"



