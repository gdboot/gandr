#include <bal/device/dt.h>
#include <bal/misc.h>
#include <gd_tree.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <libfdt.h>

RB_GENERATE(dt_properties, dt_property, rbnode, dt_property_cmp)
RB_GENERATE(dt_nodes,      dt_node,     rbnode, dt_node_cmp)

int dt_property_cmp(dt_property_t lhs, dt_property_t rhs)
{
    return strcmp(lhs->name, rhs->name);
}

int dt_node_cmp(dt_node_t lhs, dt_node_t rhs)
{
    return strcmp(lhs->name, rhs->name);
}

dt_node_t dt_node_alloc(dt_node_t parent, const char *name)
{
    dt_node_t n = malloc(sizeof *n + strlen(name) + 1);
    if (!n) return NULL;


    memset(n, 0, sizeof *n);
    n->name   = (char*) &n[1];
    n->parent = parent;

    strcpy((char*)n->name, name);

    if (parent) {
        RB_INSERT(dt_nodes, &parent->children, n);
    }

    return n;
}

dt_node_t dt_node_find_child(dt_node_t parent, const char *name)
{
    struct dt_node nd;
    memset(&nd, 0, sizeof nd);
    nd.name = name;
    return RB_FIND(dt_nodes, &parent->children, &nd);
}

dt_property_t dt_node_find_property(dt_node_t node, const char *name)
{
    struct dt_property pr;
    memset(&pr, 0, sizeof pr);
    pr.name = name;
    return RB_FIND(dt_properties, &node->properties, &pr);
}

bool dt_node_has_property(dt_node_t node, const char *name)
{
    return dt_node_find_property(node, name) != NULL;
}

const void *dt_node_get_property(dt_node_t node, const char *name)
{
    dt_property_t p = dt_node_find_property(node, name);
    if (p) {
        return p->value;
    } else return NULL;
}

dt_property_t dt_node_set_property(
    dt_node_t   node,
    const char *name,
    const void *value,
    size_t      len)
{
    dt_property_t p = dt_node_find_property(node, name);
    if (!p) {
        void *vp = malloc(len);
        if (!vp) return NULL;

        p = malloc(sizeof(*p) + strlen(name) + 1);
        if (!p) {
            free(vp);
            return NULL;
        }

        memset(p, 0, sizeof *p);
        p->name = (char*) &p[1];
        strcpy((char*) p->name, name);
        p->value = vp;

        RB_INSERT(dt_properties, &node->properties, p);
    } else if (p->value_len != len) {
        void *vp = realloc(p->value, len);
        if (!vp) return NULL;
        p->value = vp;
    }

    p->value_len = len;
    memcpy(p->value, value, len);

    return p;
}

bool dt_node_get_reg_range(
    dt_node_t  node,
    unsigned   idx,
    uintptr_t *ptr,
    size_t    *sz)
{
    dt_property_t prop = dt_node_find_property(node, "regs");
    if(!prop)
        return false;

    uint32_t acells = dt_node_get_address_cells(node->parent);
    uint32_t scells = dt_node_get_size_cells(node->parent);
    uint32_t cells  = acells + scells;

    unsigned baseIdx = idx * cells;
    if ((baseIdx + cells) * sizeof(uint32_t) > prop->value_len)
        return false;

    const uint32_t *p = prop->value;

    if (acells == 1) {
        *ptr = fdt32_to_cpu(p[baseIdx + 0]);
    } else if (acells == 2) {
        uint64_t v;
        memcpy(&v, &p[baseIdx + 0], sizeof v);
        v = fdt64_to_cpu(v);

        if (v > UINTPTR_MAX) {
            panic("Address out of range (0x%" PRIx64 ") on %s",
                v, node->name);
        }

        *ptr = v;
    }

    if (scells == 1) {
        *ptr = fdt32_to_cpu(p[acells]);
    } else if (scells == 2) {
        uint64_t v;
        memcpy(&v, &p[baseIdx + 0], sizeof v);
        v = fdt64_to_cpu(v);

        if (v > SIZE_MAX) {
            panic("Size out of range (0x%" PRIx64 ") on %s",
                v, node->name);
        }

        *sz = v;
    }

    return true;
}

unsigned dt_node_get_reg_count(dt_node_t node)
{
    dt_property_t prop = dt_node_find_property(node, "regs");
    if(!prop)
        return false;

    uint32_t acells = dt_node_get_address_cells(node->parent);
    uint32_t scells = dt_node_get_size_cells(node->parent);
    uint32_t cells  = acells + scells;

    if ((prop->value_len % (cells * sizeof(uint32_t))) != 0) {
        panic("dt: node \"%s\": <regs> has size %" PRIu32 " which isn't "
            "divisible by %" PRIu32 " cells\n", node->name, prop->value_len, cells);
    }

    return prop->value_len / (sizeof(uint32_t) * cells);
}

uint32_t dt_node_get_address_cells(dt_node_t node)
{
    dt_property_t p;
    if (!(p = dt_node_find_property(node, "#address_cells")))
        panic("Node without address cells");

    return dt_property_get_uint32(p);
}

uint32_t dt_node_get_size_cells(dt_node_t node)
{
    dt_property_t p;
    if (!(p = dt_node_find_property(node, "#size_cells")))
        panic("Node without size cells");

    return dt_property_get_uint32(p);
}

uint32_t dt_property_get_uint32(dt_property_t prop)
{
    uint32_t v;
    if (prop->value_len != sizeof v) {
        panic("dt_property_get_uint32: property (%s) not 32-bit", prop->name);
    }

    memcpy(&v, prop->value, sizeof v);
    return fdt32_to_cpu(v);
}

uint64_t dt_property_get_uint64(dt_property_t prop)
{
    uint64_t v;
    if (prop->value_len != sizeof v) {
        panic("dt_property_get_uint64: property (%s) not 64-bit", prop->name);
    }

    memcpy(&v, prop->value, sizeof v);
    return fdt64_to_cpu(v);
}
