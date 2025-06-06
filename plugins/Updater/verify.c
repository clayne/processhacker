/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     dmex    2016-2019
 *     jxy-s   2024
 *
 */

#include "updater.h"

static CONST UCHAR UpdaterTrustedPublicKeyRelease[] =
{
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00,
    0x07, 0xA4, 0x51, 0xD8, 0xD2, 0xA6, 0xC8, 0x29,
    0x25, 0x66, 0x83, 0x33, 0x6D, 0x66, 0x12, 0xDF,
    0x01, 0xDA, 0x06, 0x5A, 0x2D, 0xFA, 0x7C, 0x9B,
    0x16, 0xFC, 0xA0, 0xA2, 0xB5, 0x2D, 0xAD, 0x2E,
    0xD7, 0x17, 0xD2, 0x1B, 0x44, 0x1A, 0x32, 0x20,
    0x1B, 0x21, 0xDF, 0x4F, 0x0C, 0x28, 0xA9, 0x80,
    0x2E, 0x4B, 0x0B, 0xED, 0xCA, 0xBE, 0x61, 0xB6,
    0x37, 0x62, 0xAB, 0xA5, 0x7E, 0xB5, 0xA4, 0x3D,
};

static CONST UCHAR UpdaterTrustedPublicKeyPreview[] =
{
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00,
    0x66, 0x85, 0x77, 0x37, 0xAC, 0x02, 0x77, 0x9A,
    0x29, 0x05, 0xA8, 0x36, 0xBE, 0xF2, 0x6A, 0x54,
    0x33, 0xA7, 0xAE, 0xD5, 0x45, 0x7B, 0xB7, 0x2D,
    0x16, 0xBB, 0x4A, 0x7F, 0x1F, 0x24, 0x97, 0x52,
    0xCD, 0x5F, 0xDE, 0xA4, 0xF4, 0xC9, 0x0E, 0x9C,
    0xA9, 0xE2, 0x48, 0xF3, 0xE2, 0x9C, 0x02, 0x88,
    0xAB, 0xF5, 0xA7, 0xFB, 0x9E, 0x65, 0x46, 0xBF,
    0x1C, 0x02, 0xDC, 0x7C, 0x6E, 0x0C, 0x89, 0xA8,
};

static CONST UCHAR UpdaterTrustedPublicKeyCanary[] =
{
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00,
    0x42, 0xC8, 0x12, 0x8B, 0x37, 0x36, 0x60, 0x0D,
    0xB7, 0x5C, 0x6D, 0x80, 0xB0, 0x1A, 0xBE, 0x74,
    0xFF, 0x29, 0xBF, 0x6E, 0xD2, 0x05, 0x7D, 0x6E,
    0x54, 0x44, 0x23, 0xB0, 0x35, 0x13, 0xB5, 0xBF,
    0xFE, 0xE8, 0xCE, 0xF8, 0x3F, 0x3D, 0xED, 0x95,
    0x0E, 0x12, 0x53, 0xF9, 0xE9, 0x3B, 0x4D, 0x3D,
    0xA8, 0xC3, 0xA0, 0xD1, 0xCD, 0x06, 0xD5, 0xEC,
    0x66, 0x1B, 0x1B, 0x20, 0xE7, 0xFF, 0x6A, 0x46,
};

static CONST UCHAR UpdaterTrustedPublicKeyDeveloper[] =
{
	0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00,
    0xD1, 0x77, 0x0B, 0xBF, 0x8C, 0xA3, 0xF6, 0x20,
    0x60, 0x29, 0x3E, 0x70, 0xE8, 0xC3, 0x1B, 0x10,
	0xA4, 0x42, 0x21, 0xAD, 0x73, 0x8B, 0x8A, 0x31,
    0x3D, 0xC0, 0xD0, 0x8C, 0xD5, 0x1C, 0xC7, 0x33,
    0xA2, 0x00, 0x20, 0x0E, 0x24, 0xB5, 0x1A, 0xC8,
	0xC8, 0xDA, 0xCF, 0x2E, 0x2E, 0xD5, 0x9F, 0xEF,
    0xA7, 0x89, 0xFB, 0x99, 0x94, 0x14, 0x57, 0x5C,
    0x36, 0x04, 0x44, 0x8B, 0xA2, 0x92, 0xF8, 0x0E,
};

static CONST UCHAR UpdaterTrustedPublicKeyReleaseNext[] =
{
    0x52, 0x53, 0x41, 0x31, 0x00, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xC0, 0xF3, 0xDD, 0x50, 0xEF, 0xD3, 0xBE, 0xC8, 0xE9,
    0xFA, 0xAA, 0xE5, 0xB1, 0x97, 0x0E, 0x8A, 0x43, 0xCC, 0xAE, 0x9F, 0x4B,
    0xCC, 0xA6, 0xE3, 0xD7, 0x07, 0x90, 0x4C, 0x28, 0x82, 0xA0, 0x11, 0x25,
    0x96, 0x5C, 0xEC, 0xA0, 0xFE, 0x65, 0xA1, 0x93, 0x40, 0xA9, 0xBC, 0x06,
    0x9B, 0x2C, 0x82, 0xA6, 0x81, 0xAC, 0xCE, 0x46, 0x8D, 0x69, 0x0A, 0x83,
    0x3B, 0x2C, 0x36, 0xED, 0x14, 0x29, 0x97, 0xE7, 0x41, 0x33, 0x61, 0xD9,
    0xEB, 0xD1, 0x74, 0xAC, 0x89, 0x3A, 0x1D, 0x09, 0x00, 0x13, 0x69, 0xFB,
    0x03, 0x3E, 0x60, 0xC3, 0x07, 0x93, 0x3D, 0xA5, 0xDA, 0x76, 0x1B, 0x7A,
    0x74, 0xD5, 0xC6, 0x0D, 0x0C, 0xE5, 0x05, 0x0E, 0x76, 0xE3, 0xE7, 0x71,
    0xE7, 0x37, 0x6C, 0x0B, 0xCF, 0x98, 0x1D, 0x22, 0x35, 0x77, 0x4B, 0x3E,
    0x92, 0x6D, 0xE1, 0x96, 0xBA, 0x4C, 0x93, 0x69, 0x4F, 0x6C, 0x86, 0xD6,
    0xF3, 0x51, 0xCD, 0xEA, 0xB5, 0xF9, 0x81, 0x75, 0x74, 0xD6, 0xF4, 0x68,
    0x7F, 0x5B, 0x71, 0x4D, 0x0F, 0xE2, 0x01, 0x62, 0x76, 0x5C, 0xF8, 0xA8,
    0x45, 0xD7, 0x33, 0xC9, 0xF4, 0x1E, 0xDE, 0x9C, 0x92, 0xF1, 0x14, 0x39,
    0x4A, 0x45, 0xA7, 0xFD, 0x5D, 0xB5, 0xBA, 0x2B, 0x30, 0x49, 0x6D, 0xF4,
    0xCA, 0x35, 0x3D, 0x5C, 0xEC, 0x27, 0x20, 0xAE, 0x4B, 0x39, 0x9B, 0xAD,
    0xFC, 0x97, 0xCF, 0x2B, 0xAB, 0x47, 0x48, 0x28, 0x80, 0xE7, 0x04, 0x40,
    0x48, 0xDE, 0xE4, 0x5D, 0x05, 0xB9, 0x89, 0xDD, 0xF2, 0x5A, 0xE4, 0xB1,
    0xEA, 0x70, 0x00, 0xB5, 0x63, 0x93, 0x99, 0x52, 0x5B, 0x19, 0x06, 0xFB,
    0xAE, 0x2B, 0x36, 0x60, 0x06, 0x1E, 0xB2, 0x3A, 0xD4, 0xE1, 0x3D, 0x47,
    0x10, 0xD0, 0x4B, 0x8D, 0x94, 0xC2, 0x0D, 0xB2, 0xAD, 0x77, 0x36, 0x12,
    0xCC, 0x99, 0x2B, 0x2B, 0xCB, 0x18, 0xDF, 0x49, 0x4F, 0x58, 0x95, 0x29,
    0xE6, 0x7C, 0xA2, 0x69, 0x60, 0x79, 0xCB, 0x73, 0xD0, 0x21, 0x8E, 0xE7,
    0x4B, 0x3F, 0xEC, 0x02, 0xED, 0x93, 0xBC, 0x6C, 0x7C, 0xEF, 0x6D, 0xA6,
    0x9B, 0xB1, 0x79, 0x8A, 0x02, 0x0D, 0x3B, 0x78, 0x2A, 0x18, 0xD7, 0x67,
    0xF7, 0x22, 0xD6, 0x75, 0x5D, 0xEC, 0xCB, 0x90, 0x6A, 0x13, 0x67, 0x82,
    0x63, 0xA3, 0xBD, 0x6B, 0x63, 0x7C, 0xF8, 0xF7, 0x53, 0xD3, 0xD8, 0x20,
    0xB7, 0xC9, 0xB7, 0xEC, 0xEE, 0x21, 0x0F, 0x51, 0x6D, 0x91, 0xDD, 0xEF,
    0x6B, 0x1B, 0xA6, 0xE6, 0x89, 0xF9, 0x41, 0x65, 0xCD, 0x9A, 0x91, 0x92,
    0xE9, 0xA3, 0x6F, 0x30, 0x25, 0x5D, 0x77, 0xCF, 0x2B, 0x38, 0x0E, 0x87,
    0x40, 0x8D, 0x49, 0x99, 0x89, 0x44, 0xC1, 0x59, 0x1D, 0x72, 0x57, 0x6F,
    0x37, 0xA1, 0x2B, 0x07, 0x7F, 0x09, 0x9D, 0xCE, 0xE6, 0x9C, 0x2B, 0x94,
    0xB9, 0x2B, 0xC9, 0xA3, 0xC2, 0x25, 0xAE, 0x88, 0x59, 0x21, 0x53, 0x56,
    0x14, 0x19, 0xFA, 0xD9, 0xE4, 0x8F, 0xB7, 0x74, 0xBD, 0x46, 0x2E, 0x11,
    0x3B, 0x45, 0xC0, 0xD6, 0x90, 0x46, 0x55, 0xEE, 0xC7, 0xD4, 0x3E, 0xE8,
    0xD7, 0x93, 0xE9, 0xD5, 0x12, 0x65, 0xCB, 0x83, 0xEA, 0xE8, 0x11, 0x65,
    0xF0, 0x4A, 0x90, 0xA7, 0xB9, 0xBB, 0xC1, 0x66, 0xA4, 0x7D, 0x84, 0x8B,
    0x1B, 0xDB, 0x08, 0x63, 0x46, 0x6F, 0x0B, 0xD1, 0xEB, 0xCD, 0x4F, 0xF0,
    0x2B, 0x49, 0x32, 0x81, 0x4C, 0x5F, 0xDD, 0x36, 0xE8, 0xEE, 0xE1, 0x38,
    0xB1, 0x4B, 0x57, 0x4D, 0x46, 0x03, 0x40, 0x5A, 0x7F, 0xE5, 0x60, 0xFD,
    0xD1, 0xEC, 0x41, 0xD8, 0x15, 0xDD, 0xA0, 0xD0, 0x13, 0x31, 0xEB, 0xF9,
    0xFD, 0x75, 0x0A, 0xCF, 0x34, 0xE1, 0xC2, 0x26, 0x63, 0xEA, 0xBC, 0xA7,
    0xBF, 0x9A, 0x0E, 0x34, 0xA6, 0x72, 0xBE, 0xF1, 0x78, 0xF6, 0xB1
};

static CONST UCHAR UpdaterTrustedPublicKeyPreviewNext[] =
{
    0x52, 0x53, 0x41, 0x31, 0x00, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xAB, 0xE9, 0xD0, 0x7A, 0xC9, 0x82, 0x64, 0x18, 0xC3,
    0x23, 0x54, 0xF4, 0xCE, 0xED, 0xB6, 0x1A, 0x9E, 0xF3, 0x1C, 0x3C, 0x70,
    0x57, 0x6D, 0xE9, 0x0C, 0x9B, 0x6E, 0x05, 0x1A, 0x0B, 0x65, 0xA1, 0xB0,
    0x59, 0x2D, 0xA5, 0xEB, 0xD8, 0x21, 0x7E, 0x1E, 0x21, 0xAC, 0xE9, 0x3B,
    0x5C, 0xA4, 0x0B, 0xD1, 0xAA, 0x6B, 0x29, 0xFB, 0xA9, 0xD7, 0xD8, 0xDB,
    0xC3, 0x2F, 0xAE, 0x33, 0x6A, 0x34, 0x73, 0xD9, 0x28, 0x3A, 0x8D, 0xF0,
    0x5C, 0xF3, 0x50, 0x5D, 0xD0, 0xBE, 0xEC, 0xC6, 0x4C, 0x7B, 0x86, 0xAC,
    0xF0, 0x5D, 0x60, 0x2C, 0x6A, 0xAB, 0x8E, 0x97, 0x11, 0x6F, 0x5E, 0x28,
    0x63, 0x89, 0x3C, 0xF9, 0x82, 0x36, 0x32, 0x45, 0x7F, 0xB7, 0x0E, 0x38,
    0xDD, 0x25, 0x31, 0xCB, 0x0E, 0x1F, 0xA3, 0x7F, 0x4C, 0xF8, 0x21, 0xC3,
    0x42, 0xC9, 0xBD, 0x5F, 0x7C, 0x07, 0x1D, 0x0C, 0xE3, 0x65, 0xF9, 0x9D,
    0x2A, 0x34, 0x92, 0x4B, 0x81, 0x99, 0x9B, 0x5C, 0x35, 0x39, 0x90, 0xC0,
    0xC7, 0x1C, 0x4E, 0x8D, 0x30, 0x40, 0x07, 0x76, 0x5D, 0x6E, 0x02, 0x08,
    0x19, 0xB9, 0x8F, 0x25, 0x7E, 0xE0, 0x24, 0xD7, 0xD4, 0x4E, 0xBB, 0xEC,
    0x67, 0xDA, 0x29, 0xC4, 0xD4, 0x4E, 0xE1, 0xBB, 0x8D, 0xA0, 0xCD, 0x01,
    0xFA, 0x5F, 0x99, 0x78, 0x36, 0xE2, 0x97, 0x13, 0x03, 0x24, 0x56, 0x49,
    0x20, 0x92, 0x8A, 0xCD, 0x83, 0x39, 0xA3, 0x78, 0x4D, 0xB6, 0x2D, 0xF0,
    0x57, 0xC7, 0xAC, 0xBA, 0x2D, 0x48, 0xE8, 0x41, 0x35, 0xAC, 0xD1, 0x0D,
    0xAC, 0x85, 0x01, 0x56, 0x53, 0x84, 0x6B, 0x4A, 0xA1, 0x7C, 0xB3, 0x47,
    0xBB, 0xF4, 0x99, 0x49, 0x7F, 0x1E, 0x84, 0xFA, 0xF1, 0x19, 0xC2, 0xBD,
    0x83, 0xF0, 0xB7, 0xEA, 0x50, 0x9F, 0x4B, 0x7F, 0x13, 0x22, 0xA2, 0x54,
    0x69, 0xEC, 0x04, 0x64, 0x8E, 0xC3, 0xFB, 0xC4, 0xAB, 0x75, 0x03, 0x14,
    0xB8, 0x6B, 0x3C, 0xB7, 0xE2, 0x41, 0x70, 0x1E, 0x5C, 0x56, 0xEA, 0x39,
    0xE3, 0x79, 0x61, 0xA4, 0x54, 0xB7, 0x6C, 0x54, 0x26, 0x4D, 0x2B, 0x7D,
    0x61, 0x6E, 0x17, 0xF8, 0x3F, 0x9C, 0x95, 0x8E, 0x9E, 0x2D, 0x0F, 0x5F,
    0x0F, 0xE7, 0x9E, 0xF7, 0x93, 0x82, 0x23, 0x27, 0x57, 0xED, 0x8A, 0xDE,
    0x98, 0xE2, 0x85, 0x48, 0xC8, 0x7B, 0xDA, 0xDA, 0xB8, 0xEF, 0x7F, 0xC9,
    0x1D, 0x9E, 0x0B, 0x3E, 0x10, 0xF4, 0xC6, 0x91, 0x60, 0xD1, 0x3E, 0xD1,
    0x9F, 0x6C, 0xD1, 0xB3, 0x67, 0x34, 0x91, 0x11, 0x74, 0xE2, 0x35, 0x5E,
    0xC4, 0xFF, 0x10, 0x12, 0xE8, 0x03, 0x9D, 0x94, 0xBD, 0x2D, 0x8C, 0x1F,
    0x52, 0x88, 0xA6, 0x7B, 0xDE, 0x42, 0x63, 0xE5, 0xBF, 0xE7, 0x00, 0x3A,
    0x18, 0x59, 0xB0, 0xDF, 0xF6, 0x50, 0x3A, 0x42, 0x3C, 0xC3, 0x4B, 0x2A,
    0x42, 0xAB, 0xB3, 0xBB, 0xA3, 0xE0, 0x58, 0x24, 0x71, 0xD0, 0x14, 0x5D,
    0x38, 0xF2, 0xB0, 0x63, 0x10, 0xC7, 0xE1, 0xC9, 0x59, 0xD1, 0xF0, 0x1D,
    0xED, 0xF4, 0x5C, 0xB1, 0xAA, 0x47, 0xBF, 0xC3, 0x5B, 0x34, 0x2F, 0xD2,
    0xB4, 0x99, 0x21, 0x4D, 0xEC, 0x11, 0x30, 0x43, 0x33, 0x43, 0x90, 0x42,
    0xE7, 0xD5, 0x73, 0x40, 0xC3, 0xA7, 0x77, 0xA0, 0x3C, 0xCE, 0x52, 0x66,
    0x8C, 0xBE, 0xCD, 0x03, 0xA4, 0xBF, 0x31, 0x8C, 0xBE, 0x0C, 0x85, 0xE5,
    0xE9, 0x66, 0x5C, 0x7D, 0x49, 0x9F, 0x2B, 0x95, 0x29, 0x69, 0x4F, 0xFC,
    0x1E, 0x57, 0x0B, 0xDB, 0xDE, 0x6C, 0xE7, 0x29, 0xC5, 0xEA, 0x72, 0x9F,
    0x9D, 0x98, 0x7B, 0x7A, 0x02, 0x51, 0x6A, 0x8A, 0x1C, 0x05, 0xDC, 0x4D,
    0x16, 0x50, 0x0E, 0x5D, 0x0D, 0xD1, 0x55, 0xFB, 0x88, 0x4E, 0x70, 0xD2,
    0x56, 0x83, 0x67, 0xAB, 0x3D, 0xC5, 0x94, 0x64, 0x4A, 0xE0, 0x09
};

static CONST UCHAR UpdaterTrustedPublicKeyCanaryNext[] =
{
    0x52, 0x53, 0x41, 0x31, 0x00, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xE3, 0xAC, 0xCA, 0x48, 0x6C, 0xFC, 0xAE, 0xE9, 0x80,
    0x4B, 0xF3, 0x23, 0xDA, 0x1B, 0xDD, 0x0B, 0x7E, 0xCA, 0xA3, 0xA2, 0xC3,
    0x5C, 0xF8, 0x4B, 0x7D, 0x81, 0xD2, 0x6E, 0xE4, 0x16, 0xFD, 0xAF, 0x96,
    0xCB, 0x70, 0xE4, 0x0D, 0xEA, 0xDE, 0xF1, 0x8B, 0xBD, 0x3D, 0xB4, 0xD3,
    0xEB, 0x04, 0x17, 0x70, 0xD8, 0xE4, 0x0C, 0xFD, 0xA4, 0x4A, 0xED, 0x30,
    0xD6, 0x92, 0xC9, 0x97, 0x5E, 0x0B, 0x15, 0x52, 0x5D, 0x1E, 0x54, 0x30,
    0x02, 0x22, 0x86, 0xA3, 0xFD, 0xA1, 0xF8, 0xF5, 0x5C, 0x73, 0x94, 0x4A,
    0x07, 0xCD, 0x51, 0x10, 0xF8, 0x1E, 0x79, 0xC4, 0x14, 0x97, 0x9C, 0x0C,
    0x6C, 0x3C, 0x76, 0x04, 0x76, 0x2E, 0xD3, 0x48, 0x21, 0xE6, 0x12, 0xB7,
    0xA8, 0x86, 0xAD, 0xFB, 0x48, 0x3C, 0x3E, 0x74, 0xA5, 0xF4, 0xE1, 0x97,
    0x96, 0x51, 0x5A, 0x60, 0xC7, 0xC1, 0xFA, 0xF4, 0xCF, 0x76, 0xE5, 0x3C,
    0xCA, 0xB7, 0x1D, 0xA6, 0xAC, 0x4C, 0x2A, 0x85, 0xEB, 0x15, 0x2C, 0x3F,
    0x51, 0x91, 0x62, 0x92, 0x4E, 0x17, 0xB1, 0x70, 0x47, 0xDE, 0x8E, 0x9C,
    0xE9, 0xAD, 0xC2, 0xD0, 0x51, 0x01, 0xCB, 0x73, 0x3B, 0x74, 0x8F, 0xD8,
    0xBE, 0x71, 0x8F, 0x7F, 0xED, 0x02, 0x83, 0x8C, 0x2D, 0xAB, 0x0E, 0xF4,
    0x72, 0xF4, 0xD4, 0x31, 0xB0, 0xA2, 0x22, 0xEE, 0xC0, 0xC8, 0xD9, 0xC2,
    0x0D, 0x76, 0x3D, 0x1F, 0xBF, 0x91, 0x36, 0x0D, 0xCF, 0x5E, 0x5B, 0x50,
    0x63, 0x79, 0xB2, 0x97, 0xC6, 0x47, 0xDE, 0xFE, 0xF2, 0xE2, 0x95, 0x51,
    0xAD, 0x04, 0x1F, 0x30, 0xEA, 0x10, 0xB7, 0x5C, 0xB8, 0x05, 0xF1, 0xBD,
    0x5E, 0x69, 0xDE, 0x82, 0xDD, 0x02, 0x9C, 0x66, 0x95, 0x65, 0x65, 0x0A,
    0x58, 0x06, 0x1E, 0x2B, 0x7B, 0xA4, 0xA0, 0x61, 0x1F, 0xE1, 0x99, 0x0F,
    0xDA, 0x7D, 0x05, 0xC0, 0xC8, 0xA9, 0x19, 0x43, 0x05, 0xCD, 0x79, 0xCF,
    0xE8, 0x69, 0x81, 0xD2, 0xB8, 0xA0, 0x43, 0xE8, 0xDA, 0xA2, 0xB0, 0x9C,
    0x56, 0x78, 0xD0, 0x8C, 0xE1, 0x3D, 0x58, 0x55, 0x15, 0xA1, 0xAF, 0xC0,
    0xD5, 0x82, 0x72, 0x44, 0x2E, 0x35, 0x07, 0x2D, 0xCA, 0x0C, 0x54, 0x83,
    0x68, 0xCE, 0x14, 0x9B, 0xC7, 0x32, 0x34, 0x47, 0xB1, 0x8B, 0x20, 0x9D,
    0x66, 0x17, 0x35, 0x5B, 0x1C, 0x55, 0x59, 0xAA, 0xE8, 0xBC, 0x42, 0xF1,
    0x25, 0x9E, 0x48, 0xFE, 0xF5, 0x14, 0xCC, 0x46, 0x00, 0x31, 0x7E, 0xAE,
    0xD3, 0x68, 0x97, 0x05, 0x14, 0x9D, 0x6E, 0x47, 0xA6, 0x38, 0x77, 0x17,
    0xE1, 0xCB, 0x86, 0xE8, 0x41, 0x3E, 0x71, 0x19, 0x1C, 0xD6, 0x20, 0xCB,
    0x91, 0xAF, 0x26, 0x11, 0xC6, 0x5E, 0x72, 0x67, 0x42, 0xF0, 0x29, 0x00,
    0x86, 0xEF, 0xE6, 0x7C, 0xEC, 0xE4, 0xEB, 0xEA, 0xBF, 0xD7, 0xAE, 0xBB,
    0xE5, 0x25, 0x6B, 0x79, 0xDF, 0x4A, 0x89, 0x08, 0x2C, 0xD4, 0x3A, 0x8F,
    0xD3, 0x1E, 0x26, 0xC5, 0xDC, 0x19, 0x6E, 0x73, 0x55, 0x75, 0x47, 0xE7,
    0x4C, 0x53, 0x5B, 0x51, 0x61, 0xA8, 0xEC, 0x7C, 0xA6, 0x81, 0xA1, 0x6B,
    0x18, 0x73, 0x90, 0xD7, 0xF4, 0x70, 0x63, 0xE0, 0x16, 0x48, 0xDF, 0x1D,
    0x11, 0x0F, 0xA4, 0x6D, 0xFE, 0x97, 0x40, 0x10, 0xCF, 0xDE, 0x02, 0x1B,
    0x44, 0x7C, 0x45, 0x94, 0xF5, 0xF6, 0x91, 0xFE, 0x0A, 0x26, 0xD0, 0x6A,
    0xCD, 0xAC, 0x79, 0xCD, 0x65, 0x90, 0x20, 0x9A, 0xC4, 0x4D, 0x06, 0xE0,
    0x29, 0x44, 0xCB, 0xF0, 0x8E, 0x05, 0x58, 0x1B, 0xDE, 0xAD, 0x55, 0xD7,
    0xD2, 0x13, 0x86, 0xD3, 0xD0, 0x8B, 0xB5, 0x5C, 0xFD, 0xC0, 0x14, 0x74,
    0x1F, 0x98, 0xF5, 0x50, 0xD7, 0x79, 0x3E, 0x06, 0xCA, 0xF6, 0xFB, 0xAB,
    0x89, 0x64, 0xA1, 0xA8, 0xF4, 0x92, 0xC6, 0x80, 0xC3, 0x82, 0xB1
};

static CONST UCHAR UpdaterTrustedPublicKeyDeveloperNext[] =
{
    0x52, 0x53, 0x41, 0x31, 0x00, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xAB, 0x16, 0x70, 0x22, 0x11, 0x0E, 0x2B, 0x31, 0x0E,
    0xA1, 0xE5, 0xAE, 0x72, 0x35, 0x9A, 0xCB, 0x55, 0x87, 0x18, 0x3C, 0x9F,
    0x25, 0x0B, 0xF5, 0x00, 0xF8, 0x15, 0x36, 0xC8, 0xAE, 0xE2, 0xFD, 0xAD,
    0xB7, 0x88, 0x74, 0x8C, 0xD6, 0x02, 0xF1, 0x29, 0x90, 0x2E, 0x83, 0x08,
    0xE4, 0x10, 0xF3, 0xC6, 0x21, 0x57, 0x19, 0x98, 0x58, 0x98, 0xD3, 0xD0,
    0xCD, 0x6F, 0x4F, 0x91, 0x54, 0xA2, 0xEC, 0x5A, 0xE6, 0x70, 0x44, 0xDC,
    0x09, 0x3D, 0x4B, 0xF3, 0x4A, 0x6E, 0x39, 0xCA, 0x71, 0x1F, 0x39, 0x68,
    0xE6, 0xC6, 0xBD, 0x94, 0xD2, 0x0E, 0x8C, 0xCD, 0x0A, 0x2A, 0xF3, 0xC1,
    0xC7, 0xBE, 0xF1, 0xC5, 0xD4, 0x59, 0x60, 0x19, 0x56, 0x01, 0x32, 0x20,
    0xB6, 0x60, 0xDF, 0x58, 0x52, 0x76, 0x1C, 0x82, 0x94, 0x63, 0x3E, 0xB4,
    0xEB, 0x07, 0xD9, 0xAD, 0x47, 0xE5, 0x0B, 0xC1, 0xE9, 0xA4, 0xCF, 0xF4,
    0xD3, 0x64, 0x0A, 0x73, 0xCB, 0x62, 0x93, 0x17, 0xD2, 0x7D, 0xFC, 0xE0,
    0xFA, 0x9F, 0x7B, 0x70, 0x0E, 0x28, 0xDC, 0x9F, 0x3B, 0xB3, 0x4A, 0x80,
    0x4A, 0x8F, 0x6F, 0x27, 0xDD, 0x98, 0xF2, 0x0E, 0xE7, 0x0B, 0xDD, 0x39,
    0x00, 0xDC, 0x75, 0x3B, 0xE8, 0x18, 0x92, 0x6B, 0x61, 0x1C, 0xAC, 0xC6,
    0x21, 0x34, 0x66, 0x23, 0x85, 0x89, 0x97, 0x31, 0xCD, 0x51, 0xF4, 0x7B,
    0x29, 0xD1, 0x87, 0xF6, 0xCD, 0xBD, 0x1D, 0x89, 0x77, 0x3B, 0xFA, 0xBE,
    0xEB, 0x3E, 0xBB, 0xDD, 0x89, 0x81, 0x10, 0xB5, 0x6C, 0xDA, 0x4F, 0x77,
    0xAA, 0x8E, 0x41, 0xFF, 0xEE, 0xD1, 0x34, 0xE8, 0x5F, 0x03, 0xCA, 0x84,
    0x4A, 0x4B, 0xB0, 0xCE, 0xB5, 0x98, 0x85, 0xE3, 0xC1, 0xEF, 0x08, 0x5E,
    0xB5, 0x72, 0xB0, 0x75, 0x1A, 0x3D, 0x46, 0x32, 0xC7, 0x8C, 0x44, 0x78,
    0x91, 0xFF, 0xDF, 0x0C, 0xE5, 0x0B, 0x3B, 0x41, 0xF0, 0x3F, 0x1D, 0x50,
    0x93, 0x03, 0x1B, 0x1E, 0x4E, 0xDE, 0x91, 0xC0, 0x31, 0x3F, 0x4B, 0x9A,
    0xF2, 0xE2, 0x45, 0xF9, 0x85, 0x9E, 0xF5, 0x4F, 0xD9, 0x3C, 0x4E, 0x1F,
    0xBE, 0xDA, 0x11, 0xA6, 0x41, 0xF3, 0x02, 0x52, 0x02, 0x0D, 0xF2, 0x10,
    0x51, 0x1E, 0xFC, 0x16, 0x4D, 0x36, 0xBE, 0x38, 0x9D, 0x54, 0x72, 0xC5,
    0x65, 0x29, 0x52, 0xB3, 0x5D, 0xCB, 0x25, 0xBB, 0x86, 0xA5, 0x35, 0x86,
    0x5E, 0x50, 0x10, 0x5C, 0x37, 0x7C, 0xDC, 0x9E, 0x10, 0xBD, 0x5B, 0x7C,
    0x32, 0xF4, 0x26, 0x70, 0x48, 0x48, 0x4E, 0x7D, 0x41, 0x25, 0x16, 0xC9,
    0x20, 0xB6, 0xB5, 0x21, 0xC9, 0x50, 0x4C, 0xCE, 0x81, 0x31, 0xA3, 0x57,
    0x16, 0x8B, 0xB5, 0xB0, 0x1F, 0xA3, 0x27, 0x17, 0x18, 0x7B, 0x40, 0x7E,
    0x4B, 0x0D, 0xEC, 0x52, 0x72, 0x8B, 0x0E, 0x56, 0xE3, 0x5B, 0x29, 0xAC,
    0xDA, 0xC6, 0x71, 0xF8, 0x2B, 0x90, 0x08, 0x7A, 0x51, 0x6A, 0x9C, 0xB0,
    0xA1, 0xE2, 0x55, 0x4A, 0x9C, 0x9E, 0x91, 0xFE, 0x10, 0x05, 0x15, 0x02,
    0x49, 0x9D, 0x5A, 0x9D, 0x83, 0xDA, 0xF7, 0xE5, 0x97, 0x3A, 0xB7, 0x5C,
    0x8C, 0xAB, 0x36, 0xFE, 0xF0, 0xCE, 0xB2, 0xA1, 0xA9, 0x37, 0x4A, 0x3C,
    0x6D, 0x93, 0xA9, 0x2A, 0x64, 0xDA, 0x06, 0xD6, 0x3F, 0x06, 0x7B, 0x20,
    0x1B, 0xFB, 0xB7, 0xFF, 0xE2, 0xC0, 0xAB, 0x43, 0x87, 0x59, 0x63, 0xF5,
    0xE2, 0xDD, 0x1D, 0x6F, 0x30, 0xC3, 0xD0, 0x32, 0x59, 0xD5, 0xD7, 0x2B,
    0xEC, 0x88, 0xB2, 0xEC, 0x1A, 0x78, 0x27, 0x2D, 0x56, 0x58, 0x27, 0x2A,
    0xF3, 0xAF, 0xD5, 0x9F, 0x78, 0xA3, 0x59, 0x60, 0x1C, 0x49, 0x49, 0x76,
    0x30, 0xAD, 0xB0, 0x5D, 0xFC, 0x59, 0x72, 0x3A, 0xB7, 0x2C, 0x44, 0x61,
    0x2E, 0x96, 0x68, 0x59, 0x77, 0x51, 0x2F, 0xA0, 0x0A, 0x17, 0xE9
};

static BCRYPT_PSS_PADDING_INFO UpdaterPaddingInfo =
{
    BCRYPT_SHA512_ALGORITHM,
    (512 / 8)
};

NTSTATUS UpdaterInitializeSigning(
    _Inout_ PUPDATER_SIGNING Signing,
    _In_ PUCHAR PublicKey,
    _In_ ULONG PublicKeySize,
    _In_ PCWSTR SignAlgId,
    _In_ PCWSTR SignBlobType,
    _In_ PCWSTR HashAlgId,
    _In_opt_ PVOID PaddingInfo,
    _In_ ULONG PaddingFlags
    )
{
    NTSTATUS status;
    ULONG querySize;

    // Import the trusted public key.

    status = BCryptOpenAlgorithmProvider(
        &Signing->SignAlgHandle,
        SignAlgId,
        NULL,
        0
        );

    if (!NT_SUCCESS(status))
        return status;

    status = BCryptImportKeyPair(
        Signing->SignAlgHandle,
        NULL,
        SignBlobType,
        &Signing->KeyHandle,
        PublicKey,
        PublicKeySize,
        0
        );

    if (!NT_SUCCESS(status))
        return status;

    Signing->PaddingInfo = PaddingInfo;
    Signing->PaddingFlags = PaddingFlags;

    // Open the hash algorithm and allocate memory for the hash object.

    status = BCryptOpenAlgorithmProvider(
        &Signing->HashAlgHandle,
        HashAlgId,
        NULL,
        0
        );

    if (!NT_SUCCESS(status))
        return status;

    if (!NT_SUCCESS(status = BCryptGetProperty(
        Signing->HashAlgHandle,
        BCRYPT_HASH_LENGTH,
        (PUCHAR)&Signing->HashSize,
        sizeof(ULONG),
        &querySize,
        0
        )))
    {
        return status;
    }

    if (!(Signing->Hash = PhAllocatePageZero(Signing->HashSize)))
    {
        return STATUS_NO_MEMORY;
    }

    if (!NT_SUCCESS(status = BCryptCreateHash(
        Signing->HashAlgHandle,
        &Signing->HashHandle,
        NULL,
        0,
        NULL,
        0,
        0
        )))
    {
        return status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS UpdaterInitializeHash(
    _Out_ PUPDATER_HASH_CONTEXT* Context,
    _In_ PH_RELEASE_CHANNEL Channel
    )
{
    NTSTATUS status;
    PUPDATER_HASH_CONTEXT hashContext;
    const UCHAR* publicKey;
    ULONG publicKeySize;
    const UCHAR* publicKeyNext;
    ULONG publicKeySizeNext;
    ULONG querySize;

    switch (Channel)
    {
    case PhReleaseChannel:
        publicKey = UpdaterTrustedPublicKeyRelease;
        publicKeySize = ARRAYSIZE(UpdaterTrustedPublicKeyRelease);
        publicKeyNext = UpdaterTrustedPublicKeyReleaseNext;
        publicKeySizeNext = ARRAYSIZE(UpdaterTrustedPublicKeyReleaseNext);
        break;
    case PhPreviewChannel:
        publicKey = UpdaterTrustedPublicKeyPreview;
        publicKeySize = ARRAYSIZE(UpdaterTrustedPublicKeyPreview);
        publicKeyNext = UpdaterTrustedPublicKeyPreviewNext;
        publicKeySizeNext = ARRAYSIZE(UpdaterTrustedPublicKeyPreviewNext);
        break;
    case PhCanaryChannel:
        publicKey = UpdaterTrustedPublicKeyCanary;
        publicKeySize = ARRAYSIZE(UpdaterTrustedPublicKeyCanary);
        publicKeyNext = UpdaterTrustedPublicKeyCanaryNext;
        publicKeySizeNext = ARRAYSIZE(UpdaterTrustedPublicKeyCanaryNext);
        break;
    case PhDeveloperChannel:
        publicKey = UpdaterTrustedPublicKeyDeveloper;
        publicKeySize = ARRAYSIZE(UpdaterTrustedPublicKeyDeveloper);
        publicKeyNext = UpdaterTrustedPublicKeyDeveloperNext;
        publicKeySizeNext = ARRAYSIZE(UpdaterTrustedPublicKeyDeveloperNext);
        break;
    default:
        return STATUS_UNSUCCESSFUL;
    }

    if (!(hashContext = PhAllocatePageZero(sizeof(UPDATER_HASH_CONTEXT))))
    {
        status = STATUS_NO_MEMORY;
        goto CleanupExit;
    }

    // Initializing the hash used for validation.

    if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
        &hashContext->HashAlgHandle,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        0
        )))
    {
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = BCryptGetProperty(
        hashContext->HashAlgHandle,
        BCRYPT_HASH_LENGTH,
        (PUCHAR)&hashContext->HashSize,
        sizeof(ULONG),
        &querySize,
        0
        )))
    {
        goto CleanupExit;
    }

    if (!(hashContext->Hash = PhAllocatePageZero(hashContext->HashSize)))
    {
        status = STATUS_NO_MEMORY;
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = BCryptCreateHash(
        hashContext->HashAlgHandle,
        &hashContext->HashHandle,
        NULL,
        0,
        NULL,
        0,
        0
        )))
    {
        goto CleanupExit;
    }

    // Initialize the signing used for authentication.

    if (!NT_SUCCESS(status = UpdaterInitializeSigning(
        &hashContext->Sign[UpdaterSigningGenerationCurrent],
        (PUCHAR)publicKey,
        publicKeySize,
        BCRYPT_ECDSA_P256_ALGORITHM,
        BCRYPT_ECCPUBLIC_BLOB,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        0
        )))
    {
        goto CleanupExit;
    }

    if (!NT_SUCCESS(status = UpdaterInitializeSigning(
        &hashContext->Sign[UpdaterSigningGenerationNext],
        (PUCHAR)publicKeyNext,
        publicKeySizeNext,
        BCRYPT_RSA_ALGORITHM,
        BCRYPT_RSAPUBLIC_BLOB,
        BCRYPT_SHA512_ALGORITHM,
        &UpdaterPaddingInfo,
        BCRYPT_PAD_PSS
        )))
    {
        goto CleanupExit;
    }

CleanupExit:
    if (NT_SUCCESS(status))
    {
        *Context = hashContext;
    }
    else
    {
        *Context = NULL;
        UpdaterDestroyHash(hashContext);
    }

    return status;
}

NTSTATUS UpdaterUpdateHash(
    _In_ PUPDATER_HASH_CONTEXT Context,
    _In_reads_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length
    )
{
    NTSTATUS status;

    status = BCryptHashData(Context->HashHandle, Buffer, Length, 0);

    if (!NT_SUCCESS(status))
        return status;

    for (ULONG i = 0; i < MaxUpdaterSigningGeneration; i++)
    {
        status = BCryptHashData(Context->Sign[i].HashHandle, Buffer, Length, 0);

        if (!NT_SUCCESS(status))
            return status;
    }

    return status;
}

NTSTATUS UpdaterVerifyHash(
    _In_ PUPDATER_HASH_CONTEXT Context,
    _In_ PPH_STRING Sha2Hash
    )
{
    NTSTATUS status;
    PPH_STRING sha2HexString;

    // Compute the final hash.
    status = BCryptFinishHash(
        Context->HashHandle,
        Context->Hash,
        Context->HashSize,
        0
        );

    if (!NT_SUCCESS(status))
        return status;

    if (!(sha2HexString = PhBufferToHexString(Context->Hash, Context->HashSize)))
        return STATUS_FAIL_CHECK;

    if (!PhEqualString(sha2HexString, Sha2Hash, TRUE))
    {
        PhDereferenceObject(sha2HexString);
        return STATUS_FAIL_CHECK;
    }

    PhDereferenceObject(sha2HexString);
    return status;
}

NTSTATUS UpdaterVerifySignature(
    _In_ PUPDATER_HASH_CONTEXT Context,
    _In_ PPH_STRING HexSignature
    )
{
    NTSTATUS status;
    ULONG signatureLength;
    PUCHAR signatureBuffer;

    if (!HexSignature)
        return STATUS_FAIL_CHECK;

    signatureLength = (ULONG)HexSignature->Length / sizeof(WCHAR) / 2;
    signatureBuffer = PhAllocatePageZero(signatureLength);

    if (!PhHexStringToBufferEx(&HexSignature->sr, signatureLength, signatureBuffer))
    {
        PhFreePage(signatureBuffer);
        return STATUS_FAIL_CHECK;
    }

    status = STATUS_FAIL_CHECK;

    for (ULONG i = 0; i < MaxUpdaterSigningGeneration; i++)
    {
        // Compute the final hash.
        status = BCryptFinishHash(
            Context->Sign[i].HashHandle,
            Context->Sign[i].Hash,
            Context->Sign[i].HashSize,
            0
            );

        if (!NT_SUCCESS(status))
            continue;

        // Verify the signature.
        status = BCryptVerifySignature(
            Context->Sign[i].KeyHandle,
            Context->Sign[i].PaddingInfo,
            Context->Sign[i].Hash,
            Context->Sign[i].HashSize,
            signatureBuffer,
            signatureLength,
            Context->Sign[i].PaddingFlags
            );

        if (NT_SUCCESS(status))
            break;
    }

    PhFreePage(signatureBuffer);
    return status;
}

VOID UpdaterDestroyHash(
    _Frees_ptr_opt_ PUPDATER_HASH_CONTEXT Context
    )
{
    if (Context->HashHandle)
    {
        BCryptDestroyHash(Context->HashHandle);
        Context->HashHandle = NULL;
    }

    if (Context->HashAlgHandle)
    {
        BCryptCloseAlgorithmProvider(Context->HashAlgHandle, 0);
        Context->HashAlgHandle = NULL;
    }

    if (Context->Hash)
    {
        PhFreePage(Context->Hash);
        Context->Hash = NULL;
    }

    for (ULONG i = 0; i < MaxUpdaterSigningGeneration; i++)
    {
        if (Context->Sign[i].HashAlgHandle)
        {
            BCryptCloseAlgorithmProvider(Context->Sign[i].HashAlgHandle, 0);
            Context->Sign[i].HashAlgHandle = NULL;
        }

        if (Context->Sign[i].HashHandle)
        {
            BCryptDestroyHash(Context->Sign[i].HashHandle);
            Context->Sign[i].HashHandle = NULL;
        }

        if (Context->Sign[i].KeyHandle)
        {
            BCryptDestroyKey(Context->Sign[i].KeyHandle);
            Context->Sign[i].KeyHandle = NULL;
        }

        if (Context->Sign[i].SignAlgHandle)
        {
            BCryptCloseAlgorithmProvider(Context->Sign[i].SignAlgHandle, 0);
            Context->Sign[i].SignAlgHandle = NULL;
        }

        if (Context->Sign[i].Hash)
        {
            PhFreePage(Context->Sign[i].Hash);
            Context->Sign[i].Hash = NULL;
        }
    }

    PhFreePage(Context);
}
