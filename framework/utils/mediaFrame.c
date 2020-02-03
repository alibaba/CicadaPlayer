//
// Created by 平凯 on 2017/8/18.
//
#include <stdlib.h>
#include <string.h>
#include "mediaFrame.h"

void releaseSourceMeta(Source_meta *meta)
{
    while (meta != NULL) {
        if (meta->key)
            free(meta->key);

        if (meta->value)
            free(meta->value);

        Source_meta *meta1 = meta;
        meta = meta->next;
        free(meta1);
    }
}

void releaseMeta(Stream_meta *pMeta)
{
    if (pMeta->extradata) {
        free(pMeta->extradata);
        pMeta->extradata = NULL;
    }

    if (pMeta->lang) {
        free(pMeta->lang);
        pMeta->lang = NULL;
    }

    if (pMeta->description) {
        free(pMeta->description);
        pMeta->description = NULL;
    }

    Source_meta *meta = pMeta->meta;
    releaseSourceMeta(meta);

    pMeta->meta = NULL;
}


