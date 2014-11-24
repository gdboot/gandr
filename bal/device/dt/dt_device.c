#include <bal/device/dt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static int dt_device_get_name(
    gd_device_t dev,
    char * buf,
    size_t  szBuf,
    size_t * szNeeded)
{
    int rv = 0;
    dt_node_t node;
    if ((rv = gd_device_get_dt_node(dev, &node)))
        return rv;

    *szNeeded = strlcpy(buf, node->name, szBuf);
    return 0;
}

static int dt_device_get_parent(
    gd_device_t dev,
    gd_device_t *pparent)
{
    int rv = 0;
    dt_node_t node;

    if ((rv = gd_device_get_dt_node(dev, &node)))
        return rv;

    if (node->parent) {
        *pparent = node->parent->bound_device;
        return 0;
    } else {
        return ENOENT;
    }
}

static int dt_device_get_path(
    gd_device_t dev,
    char * buf,
    size_t  szBuf,
    size_t * szNeeded)
{
    int rv = 0;
    dt_node_t node;

    if ((rv = gd_device_get_dt_node(dev, &node)))
        return rv;

    if (node->parent) {
        gd_device_t parent = node->parent->bound_device;
        if (!parent) {
            return ENOENT;
        }

        if ((rv = gd_device_get_path(parent, buf, szBuf, szNeeded)))
            return rv;
    } else {
        *szNeeded = strlcpy(buf, "/", szBuf);
    }

    *szNeeded += strlen(node->name);
    strlcat(buf, node->name, szBuf);

    return 0;
}

GD_BEGIN_IOCTL_BASE_MAP(gd_device_t, dt_base_ioctl)
    GD_MAP_DEVICE_GET_NAME_IOCTL(dt_device_get_name)
    GD_MAP_DEVICE_GET_PATH_IOCTL(dt_device_get_path)
    GD_MAP_DEVICE_GET_PARENT_IOCTL(dt_device_get_parent)
GD_END_IOCTL_BASE_MAP()
