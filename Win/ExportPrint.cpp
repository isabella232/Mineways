/*
Copyright (c) 2011, Eric Haines
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include <CommDlg.h>
#include <stdio.h>
#include <assert.h>
#include "Resource.h"
#include "ExportPrint.h"

#define IS_STL ((epd.fileType == FILE_TYPE_ASCII_STL)||(epd.fileType == FILE_TYPE_BINARY_MAGICS_STL)||(epd.fileType == FILE_TYPE_BINARY_VISCAM_STL))

static int prevPhysMaterial;
static int curPhysMaterial;

ExportPrint::ExportPrint(void)
{
}


ExportPrint::~ExportPrint(void)
{
}


void getExportPrintData(ExportFileData *pEpd)
{
    *pEpd = epd;
}

void setExportPrintData(ExportFileData *pEpd)
{
    epd = *pEpd;
    // Anything with an indeterminate state on exit needs to get set back to a real state,
    // whatever it started with, unless some new setting has forced it to be different.
    // Currently used just for OBJ export options; indeterminates are considered unchecked otherwise.
    // Since we can switch between file formats, we need to preserve the OBJ settings and not
    // have them destroyed by exporting to VRML, for example.
    origEpd = epd;
}

INT_PTR CALLBACK ExportPrint(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

int doExportPrint(HINSTANCE hInst,HWND hWnd)
{
    gOK = 0;
    DialogBox(hInst,MAKEINTRESOURCE(IDD_EXPT_VIEW),hWnd,ExportPrint);
    // did we hit cancel?
    return gOK;
}

INT_PTR CALLBACK ExportPrint(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    char changeString[EP_FIELD_LENGTH];
    //char oldString[EP_FIELD_LENGTH];
    //char currentString[EP_FIELD_LENGTH];
    UNREFERENCED_PARAMETER(lParam);

    static int focus = -1;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            focus = -1;

            // set them up
            sprintf_s(epd.minxString,EP_FIELD_LENGTH,"%d",epd.minxVal);
            sprintf_s(epd.minyString,EP_FIELD_LENGTH,"%d",epd.minyVal);
            sprintf_s(epd.minzString,EP_FIELD_LENGTH,"%d",epd.minzVal);
            sprintf_s(epd.maxxString,EP_FIELD_LENGTH,"%d",epd.maxxVal);
            sprintf_s(epd.maxyString,EP_FIELD_LENGTH,"%d",epd.maxyVal);
            sprintf_s(epd.maxzString,EP_FIELD_LENGTH,"%d",epd.maxzVal);

            sprintf_s(epd.modelHeightString,EP_FIELD_LENGTH,"%g",epd.modelHeightVal);
            sprintf_s(epd.blockSizeString,EP_FIELD_LENGTH,"%g",epd.blockSizeVal[epd.fileType]);
            sprintf_s(epd.costString,EP_FIELD_LENGTH,"%0.2f",epd.costVal);

            sprintf_s(epd.floaterCountString,EP_FIELD_LENGTH,"%d",epd.floaterCountVal);
            sprintf_s(epd.hollowThicknessString,EP_FIELD_LENGTH,"%g",epd.hollowThicknessVal[epd.fileType]);

            SetDlgItemTextA(hDlg,IDC_WORLD_MIN_X,epd.minxString);
            SetDlgItemTextA(hDlg,IDC_WORLD_MIN_Y,epd.minyString);
            SetDlgItemTextA(hDlg,IDC_WORLD_MIN_Z,epd.minzString);
            SetDlgItemTextA(hDlg,IDC_WORLD_MAX_X,epd.maxxString);
            SetDlgItemTextA(hDlg,IDC_WORLD_MAX_Y,epd.maxyString);
            SetDlgItemTextA(hDlg,IDC_WORLD_MAX_Z,epd.maxzString);

            CheckDlgButton(hDlg,IDC_CREATE_ZIP,epd.chkCreateZip[epd.fileType]);
            CheckDlgButton(hDlg,IDC_CREATE_FILES,epd.chkCreateModelFiles[epd.fileType]);

            CheckDlgButton(hDlg,IDC_RADIO_EXPORT_NO_MATERIALS,epd.radioExportNoMaterials[epd.fileType]);
            CheckDlgButton(hDlg,IDC_RADIO_EXPORT_MTL_COLORS_ONLY,epd.radioExportMtlColors[epd.fileType]);
            CheckDlgButton(hDlg,IDC_RADIO_EXPORT_SOLID_TEXTURES,epd.radioExportSolidTexture[epd.fileType]);
            CheckDlgButton(hDlg,IDC_RADIO_EXPORT_FULL_TEXTURES,epd.radioExportFullTexture[epd.fileType]);

            // OBJ options: gray out if OBJ not in use
            if ( epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ )
            {
                CheckDlgButton(hDlg, IDC_MULTIPLE_OBJECTS, epd.chkMultipleObjects);
                CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, (epd.flags & EXPT_3DPRINT) ? BST_INDETERMINATE : epd.chkIndividualBlocks);
                // if neither of the two above are checked, this one's indeterminate
                CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, (epd.chkMultipleObjects || ((epd.flags & EXPT_3DPRINT) ? false : epd.chkIndividualBlocks)) ? epd.chkMaterialPerType : BST_INDETERMINATE);
                CheckDlgButton(hDlg, IDC_MATERIAL_SUBTYPES, epd.chkMaterialSubtypes);
                CheckDlgButton(hDlg, IDC_G3D_MATERIAL, epd.chkG3DMaterial);
            }
            else
            {
                // other file formats: keep these grayed out and unselectable;
                CheckDlgButton(hDlg, IDC_MULTIPLE_OBJECTS, BST_INDETERMINATE);
                CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, BST_INDETERMINATE);
                CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_INDETERMINATE);
                CheckDlgButton(hDlg, IDC_MATERIAL_SUBTYPES, BST_INDETERMINATE);
                CheckDlgButton(hDlg, IDC_G3D_MATERIAL, BST_INDETERMINATE);
            }

            //CheckDlgButton(hDlg,IDC_MERGE_FLATTOP,epd.chkMergeFlattop);
            CheckDlgButton(hDlg,IDC_MAKE_Z_UP,epd.chkMakeZUp[epd.fileType]);
            // under certain conditions we need to make composite overlay uncheckable, i.e. if 3D printing is on, or if detailed output is off for rendering
            CheckDlgButton(hDlg, IDC_COMPOSITE_OVERLAY, ((epd.flags & EXPT_3DPRINT) || !epd.chkExportAll) ? BST_INDETERMINATE : epd.chkCompositeOverlay);
            CheckDlgButton(hDlg, IDC_CENTER_MODEL, epd.chkCenterModel);
            // these next two options are only available for rendering
            CheckDlgButton(hDlg,IDC_TREE_LEAVES_SOLID,(epd.flags & EXPT_3DPRINT)?BST_INDETERMINATE:epd.chkLeavesSolid);
            CheckDlgButton(hDlg,IDC_BLOCKS_AT_BORDERS,(epd.flags & EXPT_3DPRINT)?BST_INDETERMINATE:epd.chkBlockFacesAtBorders);
            // disallow biome color if full texture is off
            CheckDlgButton(hDlg,IDC_BIOME,epd.radioExportFullTexture[epd.fileType]?epd.chkBiome:BST_INDETERMINATE);

            CheckDlgButton(hDlg,IDC_RADIO_ROTATE_0,epd.radioRotate0);
            CheckDlgButton(hDlg,IDC_RADIO_ROTATE_90,epd.radioRotate90);
            CheckDlgButton(hDlg,IDC_RADIO_ROTATE_180,epd.radioRotate180);
            CheckDlgButton(hDlg,IDC_RADIO_ROTATE_270,epd.radioRotate270);

            CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_HEIGHT,epd.radioScaleToHeight);
            CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_MATERIAL,epd.radioScaleToMaterial);
            CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_BLOCK,epd.radioScaleByBlock);
            CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_COST,epd.radioScaleByCost);

            SetDlgItemTextA(hDlg,IDC_MODEL_HEIGHT,epd.modelHeightString);
            SetDlgItemTextA(hDlg,IDC_BLOCK_SIZE,epd.blockSizeString);
            SetDlgItemTextA(hDlg,IDC_COST,epd.costString);

            CheckDlgButton(hDlg,IDC_FILL_BUBBLES,epd.chkFillBubbles);
            CheckDlgButton(hDlg,IDC_SEAL_ENTRANCES,epd.chkFillBubbles?epd.chkSealEntrances:BST_INDETERMINATE);
            CheckDlgButton(hDlg,IDC_SEAL_SIDE_TUNNELS,epd.chkFillBubbles?epd.chkSealSideTunnels:BST_INDETERMINATE);

            CheckDlgButton(hDlg,IDC_CONNECT_PARTS,epd.chkConnectParts);
            CheckDlgButton(hDlg,IDC_CONNECT_CORNER_TIPS,epd.chkConnectParts?epd.chkConnectCornerTips:BST_INDETERMINATE);
            CheckDlgButton(hDlg,IDC_CONNECT_ALL_EDGES,epd.chkConnectParts?epd.chkConnectAllEdges:BST_INDETERMINATE);

            CheckDlgButton(hDlg,IDC_DELETE_FLOATERS,epd.chkDeleteFloaters);
            CheckDlgButton(hDlg,IDC_HOLLOW,epd.chkHollow[epd.fileType]);
            CheckDlgButton(hDlg,IDC_SUPER_HOLLOW,epd.chkHollow[epd.fileType]?epd.chkSuperHollow[epd.fileType]:BST_INDETERMINATE);
            SetDlgItemTextA(hDlg,IDC_FLOAT_COUNT,epd.floaterCountString);
            SetDlgItemTextA(hDlg,IDC_HOLLOW_THICKNESS,epd.hollowThicknessString);

            CheckDlgButton(hDlg,IDC_MELT_SNOW,epd.chkMeltSnow);

            CheckDlgButton(hDlg,IDC_EXPORT_ALL,epd.chkExportAll);
            CheckDlgButton(hDlg,IDC_FATTEN,epd.chkExportAll?epd.chkFatten:BST_INDETERMINATE);

            BOOL debugAvailable = !epd.radioExportNoMaterials[epd.fileType] && (epd.fileType != FILE_TYPE_ASCII_STL) && (epd.fileType != FILE_TYPE_SCHEMATIC);

            CheckDlgButton(hDlg,IDC_SHOW_PARTS,debugAvailable?epd.chkShowParts:BST_INDETERMINATE);
            CheckDlgButton(hDlg,IDC_SHOW_WELDS,debugAvailable?epd.chkShowWelds:BST_INDETERMINATE);

            // When handling INITDIALOG message, send the combo box a message:
            for ( int i = 0; i < MTL_COST_TABLE_SIZE; i++ )
                SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_ADDSTRING, 0, (LPARAM)gMtlCostTable[i].wname); 

            SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_SETCURSEL, epd.comboPhysicalMaterial[epd.fileType], 0);
            prevPhysMaterial = curPhysMaterial = epd.comboPhysicalMaterial[epd.fileType];

            for ( int i = 0; i < MODELS_UNITS_TABLE_SIZE; i++ )
                SendDlgItemMessage(hDlg, IDC_COMBO_MODELS_UNITS, CB_ADDSTRING, 0, (LPARAM)gUnitTypeTable[i].wname); 

            SendDlgItemMessage(hDlg, IDC_COMBO_MODELS_UNITS, CB_SETCURSEL, epd.comboModelUnits[epd.fileType], 0);
        }
        return (INT_PTR)TRUE;
    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDC_FILL_BUBBLES:
            {
                UINT isFillBubblesChecked = IsDlgButtonChecked(hDlg,IDC_FILL_BUBBLES);
                CheckDlgButton(hDlg,IDC_SEAL_ENTRANCES,isFillBubblesChecked?epd.chkSealEntrances:BST_INDETERMINATE);
                CheckDlgButton(hDlg,IDC_SEAL_SIDE_TUNNELS,isFillBubblesChecked?epd.chkSealEntrances:BST_INDETERMINATE);
            }
            break;
        case IDC_SEAL_ENTRANCES:
            {
                // all this crazy code says is "if Fill Bubbles is not checked, keep the seal entrances checkbox indeterminate;
                // if it *is* checked, then don't allow seal entrances to become indeterminate."
                UINT isChecked = IsDlgButtonChecked(hDlg, IDC_FILL_BUBBLES);
                if ( !isChecked )
                {
                    CheckDlgButton(hDlg,IDC_SEAL_ENTRANCES,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_SEAL_ENTRANCES) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_SEAL_ENTRANCES,BST_UNCHECKED);
                }
            }
            break;
        case IDC_SEAL_SIDE_TUNNELS:
            {
                UINT isChecked = IsDlgButtonChecked(hDlg,IDC_FILL_BUBBLES);
                if ( !isChecked )
                {
                    CheckDlgButton(hDlg,IDC_SEAL_SIDE_TUNNELS,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_SEAL_SIDE_TUNNELS) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_SEAL_SIDE_TUNNELS,BST_UNCHECKED);
                }
            }
            break;

        case IDC_CONNECT_PARTS:
            {
                UINT isConnectPartsChecked = IsDlgButtonChecked(hDlg,IDC_CONNECT_PARTS);
                // if connect parts turned on, then default setting for others are turned on
                CheckDlgButton(hDlg,IDC_CONNECT_CORNER_TIPS,isConnectPartsChecked?epd.chkConnectCornerTips:BST_INDETERMINATE);
                CheckDlgButton(hDlg,IDC_CONNECT_ALL_EDGES,isConnectPartsChecked?epd.chkConnectAllEdges:BST_INDETERMINATE);
            }
        case IDC_CONNECT_CORNER_TIPS:
            {
                UINT isChecked = IsDlgButtonChecked(hDlg,IDC_CONNECT_PARTS);
                if ( !isChecked )
                {
                    CheckDlgButton(hDlg,IDC_CONNECT_CORNER_TIPS,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_CONNECT_CORNER_TIPS) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_CONNECT_CORNER_TIPS,BST_UNCHECKED);
                }
            }
            break;
        case IDC_CONNECT_ALL_EDGES:
            {
                UINT isChecked = IsDlgButtonChecked(hDlg,IDC_CONNECT_PARTS);
                if ( !isChecked )
                {
                    CheckDlgButton(hDlg,IDC_CONNECT_ALL_EDGES,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_CONNECT_ALL_EDGES) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_CONNECT_ALL_EDGES,BST_UNCHECKED);
                }
            }
            break;

        case IDC_HOLLOW:
            {
                UINT isHollowChecked = IsDlgButtonChecked(hDlg,IDC_HOLLOW);
                // if hollow turned on, then default setting for superhollow is on
                CheckDlgButton(hDlg,IDC_SUPER_HOLLOW,isHollowChecked?epd.chkSuperHollow[epd.fileType]:BST_INDETERMINATE);
            }
            break;
        case IDC_SUPER_HOLLOW:
            {
                UINT isHollowChecked = IsDlgButtonChecked(hDlg,IDC_HOLLOW);
                if ( !isHollowChecked )
                {
                    CheckDlgButton(hDlg,IDC_SUPER_HOLLOW,BST_INDETERMINATE);
                }
                else
                {
                    UINT isSuperHollowIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_SUPER_HOLLOW) == BST_INDETERMINATE );
                    if ( isSuperHollowIndeterminate )
                        CheckDlgButton(hDlg,IDC_SUPER_HOLLOW,BST_UNCHECKED);
                }
            }
            break;

        case IDC_RADIO_EXPORT_NO_MATERIALS:
            // set the combo box material to white (might already be that, which is fine)
            SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_SETCURSEL, PRINT_MATERIAL_WHITE_STRONG_FLEXIBLE, 0);
            // kinda sleazy: if we go to anything but full textures, turn off exporting all objects
            // - done because full blocks of the lesser objects usually looks dumb
            CheckDlgButton(hDlg,IDC_EXPORT_ALL, BST_UNCHECKED);
            goto ChangeMaterial;

        case IDC_RADIO_EXPORT_MTL_COLORS_ONLY:
            // set the combo box material to color (might already be that, which is fine)
            SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_SETCURSEL, PRINT_MATERIAL_FULL_COLOR_SANDSTONE, 0);
            // kinda sleazy: if we go to anything but full textures, turn off exporting all objects
            CheckDlgButton(hDlg,IDC_EXPORT_ALL, BST_UNCHECKED);
            goto ChangeMaterial;

        case IDC_RADIO_EXPORT_SOLID_TEXTURES:
            // set the combo box material to color (might already be that, which is fine)
            SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_SETCURSEL, PRINT_MATERIAL_FULL_COLOR_SANDSTONE, 0);
            // kinda sleazy: if we go to anything but full textures, turn off exporting all objects
            CheckDlgButton(hDlg,IDC_EXPORT_ALL, BST_UNCHECKED);
            goto ChangeMaterial;

        case IDC_RADIO_EXPORT_FULL_TEXTURES:
            // set the combo box material to color (might already be that, which is fine)
            SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_SETCURSEL, PRINT_MATERIAL_FULL_COLOR_SANDSTONE, 0);
            goto ChangeMaterial;

        case IDC_COMBO_PHYSICAL_MATERIAL:
ChangeMaterial:
            {
                // combo box selection will change the thickness, if previous value is set to the default
                curPhysMaterial = (int)SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_GETCURSEL, 0, 0);
                if ( prevPhysMaterial != curPhysMaterial )
                {
                    //sprintf_s(oldString,EP_FIELD_LENGTH,"%g",METERS_TO_MM * mtlCostTable[prevPhysMaterial].minWall);
                    sprintf_s(changeString,EP_FIELD_LENGTH,"%g",METERS_TO_MM * gMtlCostTable[curPhysMaterial].minWall);

                    // this old code cleverly changed the value only if the user hadn't set it to something else. This
                    // is a little too clever: if the user set the value, then there was no way he could find out what
                    // a material's minimum thickness had to be when he chose the material - he'd have to restart the
                    // program. Better to force the user to set block size again if he changes the material type.
                    //GetDlgItemTextA(hDlg,IDC_BLOCK_SIZE,currentString,EP_FIELD_LENGTH);
                    //if ( strcmp(oldString,currentString) == 0)
                    SetDlgItemTextA(hDlg,IDC_BLOCK_SIZE,changeString);

                    //GetDlgItemTextA(hDlg,IDC_HOLLOW_THICKNESS,currentString,EP_FIELD_LENGTH);
                    //if ( strcmp(oldString,currentString) == 0)
                    SetDlgItemTextA(hDlg,IDC_HOLLOW_THICKNESS,changeString);

                    prevPhysMaterial = curPhysMaterial;
                }

                // if material output turned off, don't allow debug options
                BOOL colorAvailable = !IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_NO_MATERIALS) 
                    && (epd.fileType != FILE_TYPE_ASCII_STL);
                if ( colorAvailable )
                {
                    // wipe out any indeterminates
                    if ( IsDlgButtonChecked(hDlg,IDC_SHOW_PARTS) == BST_INDETERMINATE )
                    {
                        // back to state at start
                        CheckDlgButton(hDlg,IDC_SHOW_PARTS,epd.chkShowParts);
                        CheckDlgButton(hDlg,IDC_SHOW_WELDS,epd.chkShowParts);
                    }
                }
                else
                {
                    // shut them down
                    CheckDlgButton(hDlg,IDC_SHOW_PARTS,BST_INDETERMINATE);
                    CheckDlgButton(hDlg,IDC_SHOW_WELDS,BST_INDETERMINATE);
                }
                // disallow biome color if not full texture
                CheckDlgButton(hDlg,IDC_BIOME,IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_FULL_TEXTURES)?epd.chkBiome:BST_INDETERMINATE);
            }
            break;
        case IDC_BIOME:
            {
                UINT isInactive = !IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_FULL_TEXTURES);
                if ( isInactive )
                {
                    CheckDlgButton(hDlg,IDC_BIOME,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_BIOME) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_BIOME,BST_UNCHECKED);
                }
            }
            break;
        case IDC_SHOW_PARTS:
            {
                UINT isInactive = IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_NO_MATERIALS) 
                    || (epd.fileType == FILE_TYPE_ASCII_STL);
                if ( isInactive )
                {
                    CheckDlgButton(hDlg,IDC_SHOW_PARTS,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_SHOW_PARTS) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_SHOW_PARTS,BST_UNCHECKED);
                }
            }
            break;
        case IDC_SHOW_WELDS:
            {
                UINT isInactive = IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_NO_MATERIALS) 
                    || (epd.fileType == FILE_TYPE_ASCII_STL);
                if ( isInactive )
                {
                    CheckDlgButton(hDlg,IDC_SHOW_WELDS,BST_INDETERMINATE);
                }
                else
                {
                    UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_SHOW_WELDS) == BST_INDETERMINATE );
                    if ( isIndeterminate )
                        CheckDlgButton(hDlg,IDC_SHOW_WELDS,BST_UNCHECKED);
                }
            }
            break;

        case IDC_CREATE_ZIP:
            {
                // if zip off, model file export must be set to on
                if ( !IsDlgButtonChecked(hDlg,IDC_CREATE_ZIP) )
                {
                    CheckDlgButton(hDlg,IDC_CREATE_FILES,BST_CHECKED);
                }
            }
            break;

        case IDC_CREATE_FILES:
            {
                // if model off, model file export must be set to on
                if ( !IsDlgButtonChecked(hDlg,IDC_CREATE_FILES) )
                {
                    CheckDlgButton(hDlg,IDC_CREATE_ZIP,BST_CHECKED);
                }
            }
            break;


        case IDC_MULTIPLE_OBJECTS:
            if (epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ)
            {
                // if it was checked, then it goes to indeterminate, but that really means "go to unchecked"
                if (IsDlgButtonChecked(hDlg, IDC_MULTIPLE_OBJECTS) == BST_INDETERMINATE)
                {
                    // go from the indeterminate tristate to unchecked - indeterminate is not selectable
                    CheckDlgButton(hDlg,IDC_MULTIPLE_OBJECTS,BST_UNCHECKED);
                    // now adjust sub-items. Material per type is indeterminate if multiple objects is unchecked,
                    // AND individual blocks is unchecked.
                    if (IsDlgButtonChecked(hDlg, IDC_INDIVIDUAL_BLOCKS) != BST_CHECKED)
                        CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_INDETERMINATE);
                }
                else
                {
                    // checked
                    CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_UNCHECKED);
                    if (epd.flags & EXPT_3DPRINT)
                    {
                        // for 3D printing we never allow individual blocks.
                        CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, BST_INDETERMINATE);
                    }
                    else
                    {
                        // turn off individual blocks unless the user really really wants it
                        // and turns it back on.
                        CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, BST_UNCHECKED);
                    }
                }
            }
            else
            {
                CheckDlgButton(hDlg, IDC_MULTIPLE_OBJECTS, BST_INDETERMINATE);
            }
            break;
        case IDC_INDIVIDUAL_BLOCKS:
            // the indeterminate state is only for when the option is not available (i.e., 3d printing)
            if (epd.flags & EXPT_3DPRINT)
            {
                CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, BST_INDETERMINATE);
            }
            else
            {
                if (epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ)
                {
                    if (IsDlgButtonChecked(hDlg, IDC_INDIVIDUAL_BLOCKS) == BST_INDETERMINATE)
                    {
                        // go from the indeterminate tristate to unchecked - indeterminate is not selectable
                        CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, BST_UNCHECKED);
                        // now adjust sub-items. Material per type is indeterminate if multiple objects is unchecked,
                        // AND individual blocks is unchecked.
                        if (IsDlgButtonChecked(hDlg, IDC_MULTIPLE_OBJECTS) != BST_CHECKED)
                            CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_INDETERMINATE);
                    }
                    else
                    {
                        // checked, so the box below becomes active
                        CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_UNCHECKED);
                        // turn off multiple types unless the user really really wants it
                        // and turns it back on.
                        CheckDlgButton(hDlg, IDC_MULTIPLE_OBJECTS, BST_UNCHECKED);
                    }
                }
                else
                {
                    CheckDlgButton(hDlg, IDC_INDIVIDUAL_BLOCKS, BST_INDETERMINATE);
                }
            }
            break;
        case IDC_MATERIAL_PER_TYPE:
            if ((IsDlgButtonChecked(hDlg, IDC_MULTIPLE_OBJECTS) == BST_CHECKED) || (IsDlgButtonChecked(hDlg, IDC_INDIVIDUAL_BLOCKS) == BST_CHECKED))
            {
                // things are unlocked
                if (IsDlgButtonChecked(hDlg, IDC_MATERIAL_PER_TYPE) == BST_INDETERMINATE)
                {
                    CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_UNCHECKED);
                }
            }
            else
            {
                // button is not unlocked
                CheckDlgButton(hDlg, IDC_MATERIAL_PER_TYPE, BST_INDETERMINATE);
            }
            break;
        case IDC_MATERIAL_SUBTYPES:
            if (epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ)
            {
                if (IsDlgButtonChecked(hDlg, IDC_MATERIAL_SUBTYPES) == BST_INDETERMINATE)
                {
                    CheckDlgButton(hDlg, IDC_MATERIAL_SUBTYPES, BST_UNCHECKED);
                }
            }
            else
            {
                CheckDlgButton(hDlg, IDC_MATERIAL_SUBTYPES, BST_INDETERMINATE);
            }
            break;

        case IDC_G3D_MATERIAL:
            if (epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ)
            {
                if (IsDlgButtonChecked(hDlg, IDC_G3D_MATERIAL) == BST_INDETERMINATE)
                {
                    CheckDlgButton(hDlg, IDC_G3D_MATERIAL, BST_UNCHECKED);
                }
            }
            else
            {
                CheckDlgButton(hDlg, IDC_G3D_MATERIAL, BST_INDETERMINATE);
            }
            break;

        case IDC_EXPORT_ALL:
            // if printing, special warning; this is the only time we do something special for printing vs. rendering export in this code.
            if (epd.flags & EXPT_3DPRINT) {
                if (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_CHECKED)
                {
                    // depending on file format, explain problems and solutions
                    if (epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ)
                        // Sculpteo
                        MessageBox(NULL, _T("Warning: this checkbox allows tiny features to be exported for 3D printing. Some of these small bits - fences, free-standing signs - may snap off during manufacture. Fattened versions of these objects are used by default, but even these can break. Also, the edge connection and floater checkboxes have been unchecked, since these options can cause problems. Finally, the meshes for some objects have elements that can cause Sculpteo's slicer problems - either visually check the uploaded file carefully, or if you're lucky enough to have access, use netfabb to clean up the mesh."),
                        _T("Informational"), MB_OK | MB_ICONINFORMATION);
                    else if (epd.fileType == FILE_TYPE_VRML2)
                        // Shapeways
                        MessageBox(NULL, _T("Warning: this checkbox allows tiny features to be exported for 3D printing. Some of these small bits - fences, free-standing signs - may snap off during manufacture. Fattened versions of these objects are used by default, but even these can break, so Shapeways may refuse to print the model. Also, the edge connection and floater checkboxes have been unchecked, since these options can cause problems. The one bit of good news is that Shapeways' software will clean up the mesh for you, so at least any geometric inconsistencies will not cause you problems."),
                        _T("Informational"), MB_OK | MB_ICONINFORMATION);
                    else
                        MessageBox(NULL, _T("Warning: this checkbox allows tiny features to be exported for 3D printing. Some of these small bits - fences, free-standing signs - may snap off during manufacture. Fattened versions of these objects are used by default, but even these can break. Also, the edge connection and floater checkboxes have been unchecked, since these options can cause problems. Finally, the meshes for some objects have elements that can cause some 3D printer slicers problems; you might want to clean up the mesh with software such as netfabb basic on desktop or free at http://cloud.netfabb.com."),
                        _T("Informational"), MB_OK | MB_ICONINFORMATION);
                    CheckDlgButton(hDlg, IDC_FATTEN, BST_CHECKED);
                    CheckDlgButton(hDlg, IDC_DELETE_FLOATERS, BST_UNCHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_PARTS, BST_UNCHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_CORNER_TIPS, BST_INDETERMINATE);
                    CheckDlgButton(hDlg, IDC_CONNECT_ALL_EDGES, BST_INDETERMINATE);
                }
                else if (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_UNCHECKED)
                {
                    // if lesser is toggled back off, turn on the defaults
                    CheckDlgButton(hDlg, IDC_DELETE_FLOATERS, BST_CHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_PARTS, BST_CHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_CORNER_TIPS, BST_CHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_ALL_EDGES, BST_UNCHECKED);
                }
            }
            else {
                // for rendering
                if (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_CHECKED)
                {
                    // all objects has been toggled back on, so make compositing checkable, but off, which is the default for rendering
                    CheckDlgButton(hDlg, IDC_COMPOSITE_OVERLAY, BST_UNCHECKED);
                }
                else if (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_UNCHECKED)
                {
                    // definitely make compositing uncheckable at this point - full blocks mean that composite overlay will be used, vs. separate objects
                    CheckDlgButton(hDlg, IDC_COMPOSITE_OVERLAY, BST_INDETERMINATE);

                    // just to be safe:
                    CheckDlgButton(hDlg, IDC_DELETE_FLOATERS, BST_UNCHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_PARTS, BST_UNCHECKED);
                    CheckDlgButton(hDlg, IDC_CONNECT_CORNER_TIPS, BST_INDETERMINATE);
                    CheckDlgButton(hDlg, IDC_CONNECT_ALL_EDGES, BST_INDETERMINATE);
                }
            }
            // if we're turning it off, set fatten to indeterminate state
            {
                UINT isLesserChecked = IsDlgButtonChecked(hDlg,IDC_EXPORT_ALL);
                if ( !isLesserChecked )
                    CheckDlgButton(hDlg,IDC_FATTEN,BST_INDETERMINATE);
            }
            break;

        case IDC_FATTEN:
            {
                UINT isLesserChecked = IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL);
                if (!isLesserChecked)
                {
                    CheckDlgButton(hDlg, IDC_FATTEN, BST_INDETERMINATE);
                }
                else
                {
                    UINT isFattenIndeterminate = (IsDlgButtonChecked(hDlg, IDC_FATTEN) == BST_INDETERMINATE);
                    if (isFattenIndeterminate)
                        CheckDlgButton(hDlg, IDC_FATTEN, BST_UNCHECKED);
                }
            }
            break;

        case IDC_COMPOSITE_OVERLAY:
            // the indeterminate state is only for when the option is not available (i.e., 3d printing)
            if ((epd.flags & EXPT_3DPRINT) || (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_UNCHECKED))
            {
                CheckDlgButton(hDlg, IDC_COMPOSITE_OVERLAY, BST_INDETERMINATE);
            }
            else
            {
                // always go to the next state, if we'd normally go to the indeterminate (tri-value) state
                UINT isIndeterminate = (IsDlgButtonChecked(hDlg, IDC_COMPOSITE_OVERLAY) == BST_INDETERMINATE);
                if (isIndeterminate)
                    CheckDlgButton(hDlg, IDC_COMPOSITE_OVERLAY, BST_UNCHECKED);
            }
            break;

        case IDC_TREE_LEAVES_SOLID:
            // the indeterminate state is only for when the option is not available (i.e., 3d printing)
            if ( epd.flags & EXPT_3DPRINT )
            {
                CheckDlgButton(hDlg,IDC_TREE_LEAVES_SOLID,BST_INDETERMINATE);
            }
            else
            {
                UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_TREE_LEAVES_SOLID) == BST_INDETERMINATE );
                if ( isIndeterminate )
                    CheckDlgButton(hDlg,IDC_TREE_LEAVES_SOLID,BST_UNCHECKED);
            }
            break;

        case IDC_BLOCKS_AT_BORDERS:
            // the indeterminate state is only for when the option is not available (i.e., 3d printing)
            if ( epd.flags & EXPT_3DPRINT )
            {
                CheckDlgButton(hDlg,IDC_BLOCKS_AT_BORDERS,BST_INDETERMINATE);
            }
            else
            {
                UINT isIndeterminate = ( IsDlgButtonChecked(hDlg,IDC_BLOCKS_AT_BORDERS) == BST_INDETERMINATE );
                if ( isIndeterminate )
                    CheckDlgButton(hDlg,IDC_BLOCKS_AT_BORDERS,BST_UNCHECKED);
            }
            break;

        case IDC_MODEL_HEIGHT:
            // a bit sleazy: if we get focus, then get that the box is changing, change radio button to that choice.
            // There's probably a good way to do this, but I don't know it.
            // The problem is EN_CHANGE happens when IDC_BLOCK_SIZE is first set, and we don't want to do this then
            if ( HIWORD(wParam) == EN_SETFOCUS )
            {
                focus = IDC_MODEL_HEIGHT;
            }
            else if ( (HIWORD(wParam) == EN_CHANGE) && (focus == IDC_MODEL_HEIGHT) )
            {
                epd.radioScaleByBlock = epd.radioScaleToMaterial = epd.radioScaleByCost = 0;
                epd.radioScaleToHeight = 1;
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_HEIGHT,epd.radioScaleToHeight);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_MATERIAL,epd.radioScaleToMaterial);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_BLOCK,epd.radioScaleByBlock);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_COST,epd.radioScaleByCost);
            }
            break;

        case IDC_BLOCK_SIZE:
            // a bit sleazy: if we get focus, then get that the box is changing, change radio button to that choice.
            // There's probably a good way to do this, but I don't know it.
            // The problem is EN_CHANGE happens when IDC_BLOCK_SIZE is first set, and we don't want to do this then
            if ( HIWORD(wParam) == EN_SETFOCUS )
            {
                focus = IDC_BLOCK_SIZE;
            }
            else if ( (HIWORD(wParam) == EN_CHANGE) && (focus == IDC_BLOCK_SIZE) )
            {
                epd.radioScaleToHeight = epd.radioScaleToMaterial = epd.radioScaleByCost = 0;
                epd.radioScaleByBlock = 1;
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_HEIGHT,epd.radioScaleToHeight);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_MATERIAL,epd.radioScaleToMaterial);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_BLOCK,epd.radioScaleByBlock);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_COST,epd.radioScaleByCost);
            }
            break;

        case IDC_COST:
            // a bit sleazy: if we get focus, then get that the box is changing, change radio button to that choice.
            // There's probably a good way to do this, but I don't know it.
            // The problem is EN_CHANGE happens when IDC_COST is first set, and we don't want to do this then
            if ( HIWORD(wParam) == EN_SETFOCUS )
            {
                focus = IDC_COST;
            }
            else if ( (HIWORD(wParam) == EN_CHANGE) && (focus == IDC_COST) )
            {
                epd.radioScaleToHeight = epd.radioScaleToMaterial = epd.radioScaleByBlock = 0;
                epd.radioScaleByCost = 1;
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_HEIGHT,epd.radioScaleToHeight);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_TO_MATERIAL,epd.radioScaleToMaterial);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_BLOCK,epd.radioScaleByBlock);
                CheckDlgButton(hDlg,IDC_RADIO_SCALE_BY_COST,epd.radioScaleByCost);
            }
            break;

        case IDC_FLOAT_COUNT:
            // a bit sleazy: if we get focus, then get that the box is changing, change check button to that choice.
            // There's probably a good way to do this, but I don't know it.
            // The problem is EN_CHANGE happens when IDC_FLOAT_COUNT is first set, and we don't want to do this then
            if ( HIWORD(wParam) == EN_SETFOCUS )
            {
                focus = IDC_FLOAT_COUNT;
            }
            else if ( (HIWORD(wParam) == EN_CHANGE) && (focus == IDC_FLOAT_COUNT) )
            {
                epd.chkDeleteFloaters = 1;
                CheckDlgButton(hDlg,IDC_DELETE_FLOATERS,epd.chkDeleteFloaters);
            }
            break;

        case IDC_HOLLOW_THICKNESS:
            // a bit sleazy: if we get focus, then get that the box is changing, change check button to that choice.
            // There's probably a good way to do this, but I don't know it.
            // The problem is EN_CHANGE happens when IDC_HOLLOW_THICKNESS is first set, and we don't want to do this then
            if ( HIWORD(wParam) == EN_SETFOCUS )
            {
                focus = IDC_HOLLOW_THICKNESS;
            }
            else if ( (HIWORD(wParam) == EN_CHANGE) && (focus == IDC_HOLLOW_THICKNESS) )
            {
                epd.chkHollow[epd.fileType] = 1;
                CheckDlgButton(hDlg,IDC_HOLLOW,epd.chkHollow[epd.fileType]);
            }
            break;

        case IDOK:
            {
                gOK = 1;
                ExportFileData lepd;
                lepd = epd;

                // suck all the data out to a local copy
                GetDlgItemTextA(hDlg,IDC_WORLD_MIN_X,lepd.minxString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_WORLD_MIN_Y,lepd.minyString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_WORLD_MIN_Z,lepd.minzString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_WORLD_MAX_X,lepd.maxxString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_WORLD_MAX_Y,lepd.maxyString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_WORLD_MAX_Z,lepd.maxzString,EP_FIELD_LENGTH);

                lepd.chkCreateZip[lepd.fileType] = (IsDlgButtonChecked(hDlg, IDC_CREATE_ZIP) == BST_CHECKED);
                lepd.chkCreateModelFiles[lepd.fileType] = (IsDlgButtonChecked(hDlg, IDC_CREATE_FILES) == BST_CHECKED);

                lepd.radioExportNoMaterials[lepd.fileType] = IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_NO_MATERIALS);
                lepd.radioExportMtlColors[lepd.fileType] = IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_MTL_COLORS_ONLY);
                lepd.radioExportSolidTexture[lepd.fileType] = IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_SOLID_TEXTURES);
                lepd.radioExportFullTexture[lepd.fileType] = IsDlgButtonChecked(hDlg,IDC_RADIO_EXPORT_FULL_TEXTURES);

                // OBJ options
                if (epd.fileType == FILE_TYPE_WAVEFRONT_ABS_OBJ || epd.fileType == FILE_TYPE_WAVEFRONT_REL_OBJ)
                {
                    lepd.chkMultipleObjects = (IsDlgButtonChecked(hDlg, IDC_MULTIPLE_OBJECTS) == BST_CHECKED);
                    lepd.chkMaterialPerType = (IsDlgButtonChecked(hDlg, IDC_MATERIAL_PER_TYPE) == BST_CHECKED);
                    lepd.chkMaterialSubtypes = (IsDlgButtonChecked(hDlg, IDC_MATERIAL_SUBTYPES) == BST_CHECKED);
                    lepd.chkG3DMaterial = (IsDlgButtonChecked(hDlg, IDC_G3D_MATERIAL) == BST_CHECKED);
                }
                else
                {
                    // restore state - these should never get set to indeterminate
                    lepd.chkMultipleObjects = origEpd.chkMultipleObjects;
                    lepd.chkMaterialPerType = origEpd.chkMaterialPerType;
                    lepd.chkMaterialSubtypes = origEpd.chkMaterialSubtypes;
                    lepd.chkG3DMaterial = origEpd.chkG3DMaterial;
                }
                // 3D printing should never use this option.
                lepd.chkIndividualBlocks = (epd.flags & EXPT_3DPRINT) ? 0 : (IsDlgButtonChecked(hDlg, IDC_INDIVIDUAL_BLOCKS) == BST_CHECKED);

                //lepd.chkMergeFlattop = IsDlgButtonChecked(hDlg,IDC_MERGE_FLATTOP);
                lepd.chkMakeZUp[lepd.fileType] = (IsDlgButtonChecked(hDlg, IDC_MAKE_Z_UP) == BST_CHECKED);
                lepd.chkCenterModel = (IsDlgButtonChecked(hDlg, IDC_CENTER_MODEL) == BST_CHECKED);
                // if 3D printing, or if lesser blocks is off, do composite overlay, where we make a new tile (things break otherwise)
                lepd.chkCompositeOverlay = (epd.flags & EXPT_3DPRINT) ? 1 :
                    ((IsDlgButtonChecked(hDlg, IDC_COMPOSITE_OVERLAY) == BST_CHECKED) || (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_UNCHECKED));

                // solid leaves and faces at borders always true for 3D printing.
                lepd.chkLeavesSolid = (epd.flags & EXPT_3DPRINT) ? 1 : (IsDlgButtonChecked(hDlg, IDC_TREE_LEAVES_SOLID) == BST_CHECKED);
                lepd.chkBlockFacesAtBorders = (epd.flags & EXPT_3DPRINT) ? 1 : (IsDlgButtonChecked(hDlg, IDC_BLOCKS_AT_BORDERS) == BST_CHECKED);
                lepd.chkBiome = (IsDlgButtonChecked(hDlg,IDC_BIOME) == BST_CHECKED);

                lepd.radioRotate0 = IsDlgButtonChecked(hDlg,IDC_RADIO_ROTATE_0);
                lepd.radioRotate90 = IsDlgButtonChecked(hDlg,IDC_RADIO_ROTATE_90);
                lepd.radioRotate180 = IsDlgButtonChecked(hDlg,IDC_RADIO_ROTATE_180);
                lepd.radioRotate270 = IsDlgButtonChecked(hDlg,IDC_RADIO_ROTATE_270);

                lepd.radioScaleToHeight = IsDlgButtonChecked(hDlg,IDC_RADIO_SCALE_TO_HEIGHT);
                lepd.radioScaleToMaterial = IsDlgButtonChecked(hDlg,IDC_RADIO_SCALE_TO_MATERIAL);
                lepd.radioScaleByBlock = IsDlgButtonChecked(hDlg,IDC_RADIO_SCALE_BY_BLOCK);
                lepd.radioScaleByCost = IsDlgButtonChecked(hDlg,IDC_RADIO_SCALE_BY_COST);

                GetDlgItemTextA(hDlg,IDC_MODEL_HEIGHT,lepd.modelHeightString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_BLOCK_SIZE,lepd.blockSizeString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_COST,lepd.costString,EP_FIELD_LENGTH);

                lepd.chkFillBubbles = (IsDlgButtonChecked(hDlg, IDC_FILL_BUBBLES) == BST_CHECKED);
                // if filling bubbles is off, sealing entrances does nothing at all
                lepd.chkSealEntrances = lepd.chkFillBubbles ? (IsDlgButtonChecked(hDlg, IDC_SEAL_ENTRANCES) == BST_CHECKED) : 0;
                lepd.chkSealSideTunnels = lepd.chkFillBubbles ? (IsDlgButtonChecked(hDlg, IDC_SEAL_SIDE_TUNNELS) == BST_CHECKED) : 0;

                lepd.chkConnectParts = (IsDlgButtonChecked(hDlg, IDC_CONNECT_PARTS) == BST_CHECKED);
                // if connect parts is off, corner tips and edges is off
                lepd.chkConnectCornerTips = lepd.chkConnectParts ? (IsDlgButtonChecked(hDlg, IDC_CONNECT_CORNER_TIPS) == BST_CHECKED) : 0;
                lepd.chkConnectAllEdges = lepd.chkConnectParts ? (IsDlgButtonChecked(hDlg, IDC_CONNECT_ALL_EDGES) == BST_CHECKED) : 0;

                lepd.chkDeleteFloaters = (IsDlgButtonChecked(hDlg, IDC_DELETE_FLOATERS) == BST_CHECKED);

                lepd.chkHollow[epd.fileType] = (IsDlgButtonChecked(hDlg, IDC_HOLLOW) == BST_CHECKED);
                // if hollow is off, superhollow is off
                lepd.chkSuperHollow[epd.fileType] = lepd.chkHollow[epd.fileType] ? (IsDlgButtonChecked(hDlg, IDC_SUPER_HOLLOW) == BST_CHECKED) : 0;

                lepd.chkMeltSnow = (IsDlgButtonChecked(hDlg, IDC_MELT_SNOW) == BST_CHECKED);

                GetDlgItemTextA(hDlg,IDC_FLOAT_COUNT,lepd.floaterCountString,EP_FIELD_LENGTH);
                GetDlgItemTextA(hDlg,IDC_HOLLOW_THICKNESS,lepd.hollowThicknessString,EP_FIELD_LENGTH);

                lepd.chkExportAll = (IsDlgButtonChecked(hDlg, IDC_EXPORT_ALL) == BST_CHECKED);
                lepd.chkFatten = lepd.chkExportAll ? (IsDlgButtonChecked(hDlg, IDC_FATTEN) == BST_CHECKED) : 0;

                BOOL debugAvailable = !lepd.radioExportNoMaterials[lepd.fileType] && (lepd.fileType != FILE_TYPE_ASCII_STL);
                lepd.chkShowParts = debugAvailable ? (IsDlgButtonChecked(hDlg, IDC_SHOW_PARTS) == BST_CHECKED) : 0;
                lepd.chkShowWelds = debugAvailable ? (IsDlgButtonChecked(hDlg, IDC_SHOW_WELDS) == BST_CHECKED) : 0;

                lepd.comboPhysicalMaterial[lepd.fileType] = (int)SendDlgItemMessage(hDlg, IDC_COMBO_PHYSICAL_MATERIAL, CB_GETCURSEL, 0, 0);
                lepd.comboModelUnits[lepd.fileType] = (int)SendDlgItemMessage(hDlg, IDC_COMBO_MODELS_UNITS, CB_GETCURSEL, 0, 0);

                int nc;
                nc = sscanf_s(lepd.minxString,"%d",&lepd.minxVal);
                nc &= sscanf_s(lepd.minyString,"%d",&lepd.minyVal);
                nc &= sscanf_s(lepd.minzString,"%d",&lepd.minzVal);
                nc &= sscanf_s(lepd.maxxString,"%d",&lepd.maxxVal);
                nc &= sscanf_s(lepd.maxyString,"%d",&lepd.maxyVal);
                nc &= sscanf_s(lepd.maxzString,"%d",&lepd.maxzVal);

                nc &= sscanf_s(lepd.modelHeightString,"%f",&lepd.modelHeightVal);
                nc &= sscanf_s(lepd.blockSizeString,"%f",&lepd.blockSizeVal[lepd.fileType]);
                nc &= sscanf_s(lepd.costString,"%f",&lepd.costVal);

                nc &= sscanf_s(lepd.floaterCountString,"%d",&lepd.floaterCountVal);
                nc &= sscanf_s(lepd.hollowThicknessString,"%g",&lepd.hollowThicknessVal[epd.fileType]);

                // this is a bit lazy checking all errors here, there's probably a better way
                // to test as we go, but this sort of thing should be rare
                if ( nc == 0 )
                {
                    MessageBox(NULL,
                        _T("Bad (non-numeric) value detected in options dialog;\nYou need to clean up, then hit OK again."), _T("Non-numeric value error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)FALSE;
                }

                if ( lepd.radioScaleToHeight && lepd.modelHeightVal <= 0.0f )
                {
                    MessageBox(NULL,
                        _T("Model height must be a positive number;\nYou need to fix this, then hit OK again."), _T("Value error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)FALSE;
                }

                if ( lepd.radioScaleByBlock && lepd.blockSizeVal[lepd.fileType] <= 0.0f )
                {
                    MessageBox(NULL,
                        _T("Block size must be a positive number;\nYou need to fix this, then hit OK again."), _T("Value error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)FALSE;
                }

                if ( lepd.radioScaleByCost )
                {
                    // white vs. colored stuff: $1.50 vs. $3.00 handling fees, plus some minimum amount of material
                    // We need to find out the minimum amount rules for white material; colored is at
                    // http://www.shapeways.com/design-rules/full_color_sandstone, and we use the
                    // "the dimensions have to add up to 65mm" and assume a 3mm block size to give a 59mm*3mm*3mm volume
                    // minimum, times $0.75/cm^3 gives $0.40.
                    if ( lepd.costVal <= (gMtlCostTable[curPhysMaterial].costHandling+gMtlCostTable[curPhysMaterial].costMinimum) )
                    {
                        MessageBox(NULL,
                            _T("The cost must be > $1.55 for colorless, > $3.40 for color;\nYou need to fix this, then hit OK again."), _T("Value error"), MB_OK|MB_ICONERROR);
                        return (INT_PTR)FALSE;
                    }
                }

                if ( lepd.chkDeleteFloaters && lepd.floaterCountVal < 0 )
                {
                    MessageBox(NULL,
                        _T("Floating objects deletion value cannot be negative;\nYou need to fix this, then hit OK again."), _T("Value error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)FALSE;
                }

                if ( lepd.chkHollow[epd.fileType] && lepd.hollowThicknessVal[epd.fileType] < 0.0 )
                {
                    MessageBox(NULL,
                        _T("Hollow thickness value cannot be negative;\nYou need to fix this, then hit OK again."), _T("Value error"), MB_OK|MB_ICONERROR);
                    return (INT_PTR)FALSE;
                }

                // survived tests, so really use data
                epd = lepd;
            } // yes, we do want to fall through here
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
