#include "carbon.h"
#include "ptime.h"
#include "json.h"
#include "hashmap.h"

/* Reads file from disk to memory allocating needed space */
static void* read_file_to_mem_buf(const char* fpath, size_t* buf_sz)
{
    /* Check file for existence */
    FILE* f = 0;
    f = fopen(fpath, "rb");
    if (!f)
        return 0;

    /* Gather size */
    fseek(f, 0, SEEK_END);
    long file_sz = ftell(f);
    if (file_sz == -1) {
        fclose(f);
        return 0;
    }

    /* Read contents into memory buffer */
    rewind(f);
    void* data_buf = calloc(1, file_sz + 1);
    fread(data_buf, 1, file_sz, f);
    fclose(f);

    /* Store buffer size */
    if (buf_sz)
        *buf_sz = file_sz;

    return data_buf;
}

static void path_join(char* path, const char* base, const char* uri)
{
    const char* s0 = strrchr(base, '/');
    const char* s1 = strrchr(base, '\\');
    const char* slash = s0 ? (s1 && s1 > s0 ? s1 : s0) : s1;

    if (slash) {
        size_t prefix = slash - base + 1;
        strncpy(path, base, prefix);
        strcpy(path + prefix, uri);
    } else {
        strcpy(path, base);
    }
}

static void json_parse_float_arr(float* out, struct json_array_element_s* arr_e, int lim)
{
    for (int i = 0; i < lim; ++i) {
        struct json_number_s* n = arr_e->value->payload;
        out[i] = atof(n->number);
        arr_e = arr_e->next;
    }
}

static void parse_transform_component(struct json_object_s* jobj, float translation[3], float scale[3], float rotation[4])
{
    for (struct json_object_element_s* e = jobj->start; e; e = e->next) {
        struct json_array_s* ev_jar = e->value->payload;
        if (strncmp(e->name->string, "translation", e->name->string_size) == 0) {
            json_parse_float_arr(translation, ev_jar->start, 3);
        } else if (strncmp(e->name->string, "scale", e->name->string_size) == 0) {
            json_parse_float_arr(scale, ev_jar->start, 3);
        } else if (strncmp(e->name->string, "rotation", e->name->string_size) == 0) {
            json_parse_float_arr(rotation, ev_jar->start, 4);
        }
    }
}

static void parse_light_component(struct json_object_s* jobj, float color[3], float* intensity)
{
    for (struct json_object_element_s* e = jobj->start; e; e = e->next) {
        if (strncmp(e->name->string, "color", e->name->string_size) == 0) {
            if (e->value->type == json_type_array) {
                struct json_array_s* jarr = e->value->payload;
                json_parse_float_arr(color, jarr->start, 3);
            } else if (e->value->type == json_type_number) {
                struct json_number_s* jnum = e->value->payload;
                float ctmp = atof(jnum->number);
                vec3 cval = cct(ctmp);
                memcpy(color, cval.rgb, sizeof(cval));
            }
        } else if (strncmp(e->name->string, "intensity", e->name->string_size) == 0) {
            struct json_number_s* jnum = e->value->payload;
            *intensity = atof(jnum->number);
        }
    }
}

static void parse_camera_component(struct json_object_s* jobj, float position[3], float direction[3])
{
    for (struct json_object_element_s* e = jobj->start; e; e = e->next) {
        struct json_array_s* ev_jar = e->value->payload;
        if (strncmp(e->name->string, "position", e->name->string_size) == 0) {
            json_parse_float_arr(position, ev_jar->start, 3);
        } else if (strncmp(e->name->string, "direction", e->name->string_size) == 0) {
            json_parse_float_arr(direction, ev_jar->start, 3);
        }
    }
}

void carbon_setup()
{
   time_setup();
}

void carbon_load_scene_file(ecs_world_t* world, resmngr rmgr, const char* path)
{
    /* Declare component types */
    IMPORT_COMPONENTS(world);

    /* Read scene file contents */
    char* src = read_file_to_mem_buf(path, 0);
    if (!src)
        return;

    /* Parse scene file */
    struct json_value_s* root = json_parse(src, strlen(src));
    struct json_object_s* root_jo = json_value_as_object(root);

    /* Find relevant objects */
    struct json_object_s* resources_jo = json_null;
    struct json_object_s* objects_jo = json_null;
    for (struct json_object_element_s* e = root_jo->start; e; e = e->next) {
        if (strncmp(e->name->string, "resources", e->name->string_size) == 0) {
            resources_jo = json_value_as_object(e->value);
        } else if (strncmp(e->name->string, "objects", e->name->string_size) == 0) {
            objects_jo = json_value_as_object(e->value);
        }
    }

    /* Load resources, storing reference names in temporary hashmap */
    hashmap_t* rmap = hashmap_create(0, 0);
    for (struct json_object_element_s* e = resources_jo->start; e; e = e->next) {
        /* Reference name */
        const char* key = e->name->string;
        size_t key_sz = e->name->string_size;
        /* Resource path */
        const char* value = json_value_as_string(e->value)->string;
        char* fpath = calloc(1, strlen(path) + strlen(value) + 1);
        path_join(fpath, path, value);
        /* Resource loading and storing to temp hashmap */
        rid r = resmngr_model_from_gltf(rmgr, fpath);
        rid* pr = calloc(1, sizeof(r));
        memcpy(pr, &r, sizeof(r));
        hashmap_put(rmap, key, key_sz, pr);
        free(fpath);
    }

    /* Populate scene objects as per description */
    for (struct json_object_element_s* e = objects_jo->start; e; e = e->next) {
        ecs_entity_t entity = ecs_new_entity(world, 0, 0, 0);
        for (struct json_object_element_s* f = json_value_as_object(e->value)->start; f; f = f->next) {
            if (strncmp(f->name->string, "transform", f->name->string_size) == 0) {
                transform t = (transform){
                    .pose = {
                        .translation = (vec3){{0.0, 0.0, 0.0}},
                        .scale = (vec3){{1.0, 1.0, 1.0}},
                        .rotation = quat_id()
                    },
                    .dirty = 1
                };
                parse_transform_component(
                    json_value_as_object(f->value),
                    t.pose.translation.xyz,
                    t.pose.scale.xyz,
                    t.pose.rotation.xyzw
                );
                ecs_set_ptr(world, entity, transform, &t);
            } else if (strncmp(f->name->string, "model", f->name->string_size) == 0) {
                struct json_string_s* s = json_value_as_string(f->value);
                const char* key = s->string;
                size_t key_sz = s->string_size;
                rid* pr = hashmap_get(rmap, key, key_sz);
                ecs_set(world, entity, model, { .resource = pr ? *pr : RID_INVALID });
            } else if (strncmp(f->name->string, "light", f->name->string_size) == 0) {
                light l = (light){
                    .color = vec3_zero(),
                    .intensity = 0,
                };
                parse_light_component(
                    json_value_as_object(f->value),
                    l.color.xyz, &l.intensity
                );
                ecs_set_ptr(world, entity, light, &l);
            } else if (strncmp(f->name->string, "camera", f->name->string_size) == 0) {
                camera c;
                camera_defaults(&c);
                float pos[3]; float dir[3];
                parse_camera_component(
                    json_value_as_object(f->value),
                    pos, dir
                );
                camera_setpos(&c, *(vec3*)pos);
                camera_setdir(&c, *(vec3*)dir);
                ecs_set_ptr(world, entity, camera, &c);
            }
        }
    }

    /* Free allocated resources */
    void* k = 0; rid* pr;
    uintmax_t iter = HM_WALK_BEGIN;
    while ((k = hashmap_walk(rmap, &iter, 0, (void**)&pr)) != 0) {
        free(pr);
    }
    hashmap_destroy(rmap);
    free(root);
    free(src);
}
