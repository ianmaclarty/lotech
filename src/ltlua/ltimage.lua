lt.images = {}
lt.atlas_size = 1024
lt.dump_atlases = false
local atlas_textures = {}

-- Load an array of images.  Each entry in the array should be the filename of a PNG
-- file ending in .png.  The loaded images are placed in the lt.images table, indexed
-- by the filename without the .png suffix.
function lt.LoadImages(images)
    local packer = lt.ImagePacker(lt.atlas_size)
    local atlas_num = 1
    local
    function gen_atlas_texture()
        local texture
        if lt.dump_atlases then
            local dump_file = "atlas" .. atlas_num .. ".png"
            texture = lt.CreateAtlasTexture(packer, dump_file)
        else
            texture = lt.CreateAtlasTexture(packer)
        end
        table.insert(atlas_textures, texture)
        -- Add the images to lt.images.
        lt.AddPackerImagesToTable(lt.images, packer, texture)
        lt.DeleteImagesInPacker(packer)
        atlas_num = atlas_num + 1
    end

    -- Pack images into atlas textures.
    for _, file in ipairs(images) do
        local img_buf = lt.ReadImage(file)
        if not lt.PackImage(packer, img_buf) then
            -- packer full, so generate an atlas texture.
            gen_atlas_texture()
            if not lt.PackImage(packer, img_buf) then
                error("Image " .. file .. " is too large")
            end
        end
    end

    -- Pack any images left in packer into a new texture.
    if lt.ImagePackerSize(packer) > 0 then
        gen_atlas_texture()
    end

    lt.DeleteImagePacker(packer);
end

-- Clear lt.images.  The images become invalid.
function lt.ClearImages()
    for file, img in pairs(images) do
        img:Release()
    end
    for i, texture in pairs(atlas_textures) do
        lt.DeleteTexture(texture)
    end
    images = {}
    atlas_textures = {}
    images_loaded = false
end
