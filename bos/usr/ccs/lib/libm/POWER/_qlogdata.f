* @(#)61	1.1  src/bos/usr/ccs/lib/libm/POWER/_qlogdata.f, libm, bos411, 9428A410j 12/13/90 19:44:47
* COMPONENT_NAME: LIBCCNV
*
* FUNCTIONS: none
*
* ORIGINS: 55
*
*                  SOURCE MATERIAL
*
* Copyright (c) ISQUARE, Inc. 1990

*************************************************************************
*            PROGRAM: QUAD PRECISION LOGARITHM DATA TABLES              *                         *
*            AUTHOR:  ISQUARE, Inc., (V. Markstein)                     *
*            DATE:    4/3/90                                            *
*            MODIFIED:9/22/90 customized for RS/6000 performance        *
*            NOTICE:  Copyright (c) ISQUARE, Inc. 1990                  *
*************************************************************************
*************************************************************************
*    These are coefficients for the first sixteen terms of the TAYLOR   *
*    SERIES for 720720.0 * ln (1+x).                                    *
*************************************************************************
       data     coeff           / 720720.0, -360360.0,  240240.0,
     +                           -180180.0,  144144.0, -120120.0,
     +                            102960.0,  -90090.0,   80080.0,
     +                            -72072.0,   65520.0,  -60060.0,
     +                             55440.0,  -51480.0,   48048.0,
     +                            -45045.0 /
***********************************************************************
*    This is the ACCURATE TABLE for ln x.  Of each two line entry,    *
*    the first number represents the axis variable, where its log     *
*    can be represented to at least 121 bit precision by a quad word. *
*    The second number is the reciprocal of the first, and the last   *
*    two are the quad word logarithm of the first.                    *
***********************************************************************
       data     ((logtbl(i,j),i=1,4),j=0,31)   /
     +            z'3FF0200000026746',z'3FEFC07F01F74C65',
     +               z'3F7FE02A6D72E887',z'3B3F98BF5125A0D9',
     +            z'3FF06000000C15E4',z'3FEF44659E332ED4',
     +               z'3F97B91B0AC9738C',z'3B56E8A1B39F02BA',
     +            z'3FF0A0000014A543',z'3FEECC07B2DBAE11',
     +               z'3FA39E87BC7A8F8A',z'3B626302A58A48BC',
     +            z'3FF0E0000011E06B',z'3FEE573AC8E1C131',
     +               z'3FAB42DD7337D5A8',z'3B6B338C5DDA3A53',
     +            z'3FF120000013DA38',z'3FEDE5D6E3D5DDB6',
     +               z'3FB16536EFCC407C',z'3B715E8E0264EABA',
     +            z'3FF16000000A2948',z'3FED77B654A6F07C',
     +               z'3FB51B073F9BCF0A',z'3B74DB56DE56C8EF',
     +            z'3FF1A0000012B0CD',z'3FED0CB58F4FF21D',
     +               z'3FB8C345D7411583',z'3B786152587782F8',
     +            z'3FF1E000000EE3A6',z'3FECA4B3054705AF',
     +               z'3FBC5E5490310477',z'3B7C237F7657770C',
     +            z'3FF2200000153135',z'3FEC3F8F01A2F19D',
     +               z'3FBFEC9133073D1A',z'3B7FACCC73F05939',
     +            z'3FF2600000241909',z'3FEBDD2B895D49D0',
     +               z'3FC1B72AD62ADC8D',z'3B817554B20DE7D5',
     +            z'3FF2A000002BCBD4',z'3FEB7D6C3D998F3A',
     +               z'3FC371FC214B8C8E',z'3B82E934DA5043A5',
     +            z'3FF2E000000DA3AC',z'3FEB20364058E6DD',
     +               z'3FC526E5E3FE32D7',z'3B851132C8E084BC',
     +            z'3FF320000021AE4E',z'3FEAC5701A964A93',
     +               z'3FC6D60FE7FB3D90',z'3B86BC1B66F15A74',
     +            z'3FF3600000196038',z'3FEA6D01A6AD7E27',
     +               z'3FC87FA065C86E05',z'3B887653DF907BC0',
     +            z'3FF3A0000021FEB5',z'3FEA16D3F94D19C3',
     +               z'3FCA23BC20C06EFE',z'3B8A0FA93CB516A7',
     +            z'3FF3E000000D0738',z'3FE9C2D14ED3BE0F',
     +               z'3FCBC2867481747C',z'3B8BA61B516322DC',
     +            z'3FF420000026BB6E',z'3FE970E4F7DBC1DE',
     +               z'3FCD5C216C46142A',z'3B8CF4E45F70ECE3',
     +            z'3FF46000001A700A',z'3FE920FB49B04706',
     +               z'3FCEF0ADCC826F72',z'3B8ECCCFC2085FC9',
     +            z'3FF4A0000013F66B',z'3FE8D3018D1811ED',
     +               z'3FD0402594F2C209',z'3B903E905D16156B',
     +            z'3FF4E0000011F43B',z'3FE886E5F09697FA',
     +               z'3FD1058BF9E55645',z'3B90EE57086B5AEC',
     +            z'3FF5200000219250',z'3FE83C977A8C3A9F',
     +               z'3FD1C898C1CF4F30',z'3B919E40EDF99C22',
     +            z'3FF560000025844D',z'3FE7F405FCD7747F',
     +               z'3FD2895A144EDB60',z'3B926EE9301CD24B',
     +            z'3FF5A00000426EFC',z'3FE7AD2208983088',
     +               z'3FD347DD9B5D1A24',z'3B934057A2FCAF6F',
     +            z'3FF5E0000023BDD6',z'3FE767DCE40E6B97',
     +               z'3FD4043086EF39B2',z'3B93FF8C8952354B',
     +            z'3FF62000003B535A',z'3FE724287F08D1CD',
     +               z'3FD4BE5F96231467',z'3B94825CC3371E24',
     +            z'3FF66000001DA1F2',z'3FE6E1F76B24E9B5',
     +               z'3FD57677179A1CC5',z'3B956330FA60E69E',
     +            z'3FF6A0000037A0C1',z'3FE6A13CD11BCEC4',
     +               z'3FD62C82F35722D2',z'3B95FA0B10B586EE',
     +            z'3FF6E000001B76C0',z'3FE661EC6A364397',
     +               z'3FD6E08EAA78789F',z'3B96D1986621D929',
     +            z'3FF72000003ED5D5',z'3FE623FA76C53936',
     +               z'3FD792A5608B2E43',z'3B9760893B698591',
     +            z'3FF76000003610DA',z'3FE5E75BB89D6C37',
     +               z'3FD842D1DAB292E6',z'3B982832EB89C0AF',
     +            z'3FF7A0000016DE9E',z'3FE5AC056AEC6020',
     +               z'3FD8F11E877456E8',z'3B98DD6D9D2A6F65',
     +            z'3FF7E000004C046C',z'3FE571ED3C0C2377',
     +               z'3FD99D9581E3A6B3',z'3B99848C4BFDF0E3'    /
*************************************
*  Continuation of the above table  *
*************************************

       data     ((logtbl(i,j),i=1,4),j=32,63)   /
     +            z'3FE8200000393E5C',z'3FF5390948C1B475',
     +               z'BFD214456C76DD04',z'3B91F8C33DC493A4',
     +            z'3FE86000003EFB2C',z'3FF5015014CB09F5',
     +               z'BFD16B5CCB079DCA',z'3B910FAA072FC35E',
     +            z'3FE8A000003E6A46',z'3FF4CAB886F0FC55',
     +               z'BFD0C42D66BF2B99',z'3B905CD8563746EF',
     +            z'3FE8E00000477E7A',z'3FF49539E377A7F3',
     +               z'BFD01EAE556ED4C7',z'3B8F705EB9C5B7AA',
     +            z'3FE920000037DAA4',z'3FF460CBC7C8826B',
     +               z'BFCEF5ADE3C07315',z'3B8DE963957971E5',
     +            z'3FE960000031BEA8',z'3FF42D6625AD915C',
     +               z'BFCDB13DAFD99B61',z'3B8D27CBE3668C21',
     +            z'3FE9A0000031523A',z'3FF3FB013F899EFB',
     +               z'BFCC6FFBC5F9B1E6',z'3B8BE9E55F85EE97',
     +            z'3FE9E00000343918',z'3FF3C995A453BC21',
     +               z'BFCB31D856597734',z'3B8AF392D52D7571',
     +            z'3FEA200000199638',z'3FF3991C2C054DA3',
     +               z'BFC9F6C4068B3974',z'3B89EE789ACC9199',
     +            z'3FEA60000033C6EC',z'3FF3698DF3B7EB7F',
     +               z'BFC8BEAFEA3DB758',z'3B88943663D97AE8',
     +            z'3FEAA00000276244',z'3FF33AE45B3B4ABA',
     +               z'BFC7898D8486F5D7',z'3B86F513CA9E0A1C',
     +            z'3FEAE000001DE89C',z'3FF30D19011B9DF1',
     +               z'BFC6574EBDFDA066',z'3B86222AB12C0834',
     +            z'3FEB20000037A536',z'3FF2E025C024C7B9',
     +               z'BFC527E5E39B1FE9',z'3B8458D5E445BFCF',
     +            z'3FEB6000000CCE1A',z'3FF2B404ACF86B95',
     +               z'BFC3FB45A55D490D',z'3B83C23DACE1BFEC',
     +            z'3FEBA000002CA378',z'3FF288B0126ABD40',
     +               z'BFC2D1610BB7AC3B',z'3B82B40D1D2C66D4',
     +            z'3FEBE00000343A2E',z'3FF25E22705E28E9',
     +               z'BFC1AA2B7D342442',z'3B812B63558826AF',
     +            z'3FEC20000038FF08',z'3FF234567875D88C',
     +               z'BFC08598B49AD49F',z'3B7F3F8E64C28802',
     +            z'3FEC6000001B33E6',z'3FF20B470C567468',
     +               z'BFBEC7398214A4A6',z'3B7E277BA41F0AAF',
     +            z'3FECA00000119564',z'3FF1E2EF3B34BBBD',
     +               z'BFBC8858011F0A29',z'3B7C3C3DB4C961E9',
     +            z'3FECE000001996BC',z'3FF1BB4A4037367A',
     +               z'BFBA4E763FCEDEB6',z'3B7989B53403569D',
     +            z'3FED20000027E986',z'3FF1945380748B78',
     +               z'BFB8197E2DE212FA',z'3B779AB62FAC97BD',
     +            z'3FED60000019EB78',z'3FF16E068933124A',
     +               z'BFB5E95A4CB5AE64',z'3B754C627502E4A2',
     +            z'3FEDA0000012ADBC',z'3FF1485F0DFFE7A7',
     +               z'BFB3BDF5A73085C0',z'3B7355A8D9528443',
     +            z'3FEDE0000020DF02',z'3FF12358E74A54DA',
     +               z'BFB1973BD02CA8E2',z'3B711C99788FFA22',
     +            z'3FEE2000001D96D8',z'3FF0FEF010EE3E82',
     +               z'BFAEEA31BE0FD392',z'3B6E25BF730D5F04',
     +            z'3FEE6000000AA830',z'3FF0DB20A8895CA2',
     +               z'BFAAAEF2D0476EBF',z'3B6A44120CC9DB9D',
     +            z'3FEEA000001B75A4',z'3FF0B7E6EC16A042',
     +               z'BFA67C94F109A73B',z'3B653651D572AAB2',
     +            z'3FEEE0000017A242',z'3FF0953F38F457BA',
     +               z'BFA252F32E052CD8',z'3B60EC4F64C2DE39',
     +            z'3FEF2000000CF922',z'3FF073260A411C88',
     +               z'BF9C63D2EA69DB02',z'3B5C53BDE86F9708',
     +            z'3FEF6000000BAB68',z'3FF05197F7D12236',
     +               z'BF9432A9241B2F6E',z'3B53F5BE52B566FB',
     +            z'3FEFA000000E227C',z'3FF03091B51821B3',
     +               z'BF8824489FF5499F',z'3B472044A44CCF4D',
     +            1.0d0, 1.0d0, 0.0d0, 0.0d0  /
