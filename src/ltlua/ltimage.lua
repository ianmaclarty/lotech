lt.images = {}
lt.atlas_size = 1024
local images_loaded = false
local image_queue = {}
local atlas_textures = {}

-- Queue the given png file for loading.  The .png suffix should be omitted.
function lt.QueueImage(file)
    if images_loaded then
        error("Images already loaded.  Call lt.ClearImages first.", 2)
    end
    table.insert(image_queue, file)
end

-- Load queued images.
function lt.LoadImages()
    if images_loaded then
        error("Images already loaded.  Call lt.ClearImages first.", 2)
    end

    local packer = lt.ImagePacker(lt.atlas_size)

    local
    function gen_atlas_texture()
        local texture = lt.TextureFromImagePacker(packer)
        table.insert(atlas_textures, texture)
        -- Add the images to lt.images.
        lt.AddPackerImagesToTable(lt.images, packer, texture)
        lt.ClearImagePacker(packer)
    end

    -- Pack images into atlas textures.
    for _, file in ipairs(image_queue) do
        local img_buf = lt.ReadImage(file)
        if not lt.PackImage(packer, img_buf) then
            -- packer full, so generate an atlas texture.
            gen_atlas_texture()
        end
    end

    -- Pack any images left in packer into a new texture.
    if lt.ImagePackerSize(packer) > 0 then
        gen_atlas_texture()
    end

    images_loaded = true
    image_queue = {}
end

-- Clear lt.images.  The images become invalid.
function lt.ClearImages()
    for file, img in pairs(images) do
        lt.DeleteImage(img)
    end
    for i, texture in pairs(atlas_textures) do
        lt.DeleteTexture(texture)
    end
    images = {}
    atlas_textures = {}
    images_loaded = false
end
