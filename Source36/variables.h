//
//Variables.h	: Register declaration
//
/*
Disclaimer: THIS INFORMATION IS PROVIDED 'AS-IS' FOR EVALUATION PURPOSES ONLY.  
INTERSIL CORPORATION AND ITS SUBSIDIARIES ('INTERSIL') DISCLAIM ALL WARRANTIES, 
INCLUDING WITHOUT LIMITATION FITNESS FOR A PARTICULAR PURPOSE AND MERCHANTABILITY.  
Intersil provides evaluation platforms to help our customers to develop products. 
However, factors beyond Intersil's control could significantly affect Intersil 
product performance. 
It remains the customers' responsibility to verify the actual system performance.
*/

#ifndef __VARIABLES__
#define __VARIABLES__

#ifdef INTERNAL_MCU
xdata BYTE REG00	_at_ 0xc000;
xdata BYTE REG01	_at_ 0xc001;
xdata BYTE REG02	_at_ 0xc002;
xdata BYTE REG03	_at_ 0xc003;
xdata BYTE REG04	_at_ 0xc004;
xdata BYTE REG05	_at_ 0xc005;
xdata BYTE REG06	_at_ 0xc006;
xdata BYTE REG07	_at_ 0xc007;
xdata BYTE REG08	_at_ 0xc008;
xdata BYTE REG09	_at_ 0xc009;
xdata BYTE REG0a	_at_ 0xc00a;
xdata BYTE REG0b	_at_ 0xc00b;
xdata BYTE REG0c	_at_ 0xc00c;
xdata BYTE REG0d	_at_ 0xc00d;
xdata BYTE REG0e	_at_ 0xc00e;
xdata BYTE REG0f	_at_ 0xc00f;
xdata BYTE REG10	_at_ 0xc010;
xdata BYTE REG11	_at_ 0xc011;
xdata BYTE REG12	_at_ 0xc012;
xdata BYTE REG13	_at_ 0xc013;
xdata BYTE REG14	_at_ 0xc014;
xdata BYTE REG15	_at_ 0xc015;
xdata BYTE REG16	_at_ 0xc016;
xdata BYTE REG17	_at_ 0xc017;
xdata BYTE REG18	_at_ 0xc018;
xdata BYTE REG19	_at_ 0xc019;
xdata BYTE REG1a	_at_ 0xc01a;
xdata BYTE REG1b	_at_ 0xc01b;
xdata BYTE REG1c	_at_ 0xc01c;
xdata BYTE REG1d	_at_ 0xc01d;
xdata BYTE REG1e	_at_ 0xc01e;
xdata BYTE REG1f	_at_ 0xc01f;
xdata BYTE REG20	_at_ 0xc020;
xdata BYTE REG21	_at_ 0xc021;
xdata BYTE REG22	_at_ 0xc022;
xdata BYTE REG23	_at_ 0xc023;
xdata BYTE REG24	_at_ 0xc024;
xdata BYTE REG25	_at_ 0xc025;
xdata BYTE REG26	_at_ 0xc026;
xdata BYTE REG27	_at_ 0xc027;
xdata BYTE REG28	_at_ 0xc028;
xdata BYTE REG29	_at_ 0xc029;
xdata BYTE REG2a	_at_ 0xc02a;
xdata BYTE REG2b	_at_ 0xc02b;
xdata BYTE REG2c	_at_ 0xc02c;
xdata BYTE REG2d	_at_ 0xc02d;
xdata BYTE REG2e	_at_ 0xc02e;
xdata BYTE REG2f	_at_ 0xc02f;
xdata BYTE REG30	_at_ 0xc030;
xdata BYTE REG31	_at_ 0xc031;
xdata BYTE REG32	_at_ 0xc032;
xdata BYTE REG33	_at_ 0xc033;
xdata BYTE REG34	_at_ 0xc034;
xdata BYTE REG35	_at_ 0xc035;
xdata BYTE REG36	_at_ 0xc036;
xdata BYTE REG37	_at_ 0xc037;
xdata BYTE REG38	_at_ 0xc038;
xdata BYTE REG39	_at_ 0xc039;
xdata BYTE REG3a	_at_ 0xc03a;
xdata BYTE REG3b	_at_ 0xc03b;
xdata BYTE REG3c	_at_ 0xc03c;
xdata BYTE REG3d	_at_ 0xc03d;
xdata BYTE REG3e	_at_ 0xc03e;
xdata BYTE REG3f	_at_ 0xc03f;
xdata BYTE REG40	_at_ 0xc040;
xdata BYTE REG41	_at_ 0xc041;
xdata BYTE REG42	_at_ 0xc042;
xdata BYTE REG43	_at_ 0xc043;
xdata BYTE REG44	_at_ 0xc044;
xdata BYTE REG45	_at_ 0xc045;
xdata BYTE REG46	_at_ 0xc046;
xdata BYTE REG47	_at_ 0xc047;
xdata BYTE REG48	_at_ 0xc048;
xdata BYTE REG49	_at_ 0xc049;
xdata BYTE REG4a	_at_ 0xc04a;
xdata BYTE REG4b	_at_ 0xc04b;
xdata BYTE REG4c	_at_ 0xc04c;
xdata BYTE REG4d	_at_ 0xc04d;
xdata BYTE REG4e	_at_ 0xc04e;
xdata BYTE REG4f	_at_ 0xc04f;
xdata BYTE REG50	_at_ 0xc050;
xdata BYTE REG51	_at_ 0xc051;
xdata BYTE REG52	_at_ 0xc052;
xdata BYTE REG53	_at_ 0xc053;
xdata BYTE REG54	_at_ 0xc054;
xdata BYTE REG55	_at_ 0xc055;
xdata BYTE REG56	_at_ 0xc056;
xdata BYTE REG57	_at_ 0xc057;
xdata BYTE REG58	_at_ 0xc058;
xdata BYTE REG59	_at_ 0xc059;
xdata BYTE REG5a	_at_ 0xc05a;
xdata BYTE REG5b	_at_ 0xc05b;
xdata BYTE REG5c	_at_ 0xc05c;
xdata BYTE REG5d	_at_ 0xc05d;
xdata BYTE REG5e	_at_ 0xc05e;
xdata BYTE REG5f	_at_ 0xc05f;
xdata BYTE REG60	_at_ 0xc060;
xdata BYTE REG61	_at_ 0xc061;
xdata BYTE REG62	_at_ 0xc062;
xdata BYTE REG63	_at_ 0xc063;
xdata BYTE REG64	_at_ 0xc064;
xdata BYTE REG65	_at_ 0xc065;
xdata BYTE REG66	_at_ 0xc066;
xdata BYTE REG67	_at_ 0xc067;
xdata BYTE REG68	_at_ 0xc068;
xdata BYTE REG69	_at_ 0xc069;
xdata BYTE REG6a	_at_ 0xc06a;
xdata BYTE REG6b	_at_ 0xc06b;
xdata BYTE REG6c	_at_ 0xc06c;
xdata BYTE REG6d	_at_ 0xc06d;
xdata BYTE REG6e	_at_ 0xc06e;
xdata BYTE REG6f	_at_ 0xc06f;
xdata BYTE REG70	_at_ 0xc070;
xdata BYTE REG71	_at_ 0xc071;
xdata BYTE REG72	_at_ 0xc072;
xdata BYTE REG73	_at_ 0xc073;
xdata BYTE REG74	_at_ 0xc074;
xdata BYTE REG75	_at_ 0xc075;
xdata BYTE REG76	_at_ 0xc076;
xdata BYTE REG77	_at_ 0xc077;
xdata BYTE REG78	_at_ 0xc078;
xdata BYTE REG79	_at_ 0xc079;
xdata BYTE REG7a	_at_ 0xc07a;
xdata BYTE REG7b	_at_ 0xc07b;
xdata BYTE REG7c	_at_ 0xc07c;
xdata BYTE REG7d	_at_ 0xc07d;
xdata BYTE REG7e	_at_ 0xc07e;
xdata BYTE REG7f	_at_ 0xc07f;
xdata BYTE REG80	_at_ 0xc080;
xdata BYTE REG81	_at_ 0xc081;
xdata BYTE REG82	_at_ 0xc082;
xdata BYTE REG83	_at_ 0xc083;
xdata BYTE REG84	_at_ 0xc084;
xdata BYTE REG85	_at_ 0xc085;
xdata BYTE REG86	_at_ 0xc086;
xdata BYTE REG87	_at_ 0xc087;
xdata BYTE REG88	_at_ 0xc088;
xdata BYTE REG89	_at_ 0xc089;
xdata BYTE REG8a	_at_ 0xc08a;
xdata BYTE REG8b	_at_ 0xc08b;
xdata BYTE REG8c	_at_ 0xc08c;
xdata BYTE REG8d	_at_ 0xc08d;
xdata BYTE REG8e	_at_ 0xc08e;
xdata BYTE REG8f	_at_ 0xc08f;
xdata BYTE REG90	_at_ 0xc090;
xdata BYTE REG91	_at_ 0xc091;
xdata BYTE REG92	_at_ 0xc092;
xdata BYTE REG93	_at_ 0xc093;
xdata BYTE REG94	_at_ 0xc094;
xdata BYTE REG95	_at_ 0xc095;
xdata BYTE REG96	_at_ 0xc096;
xdata BYTE REG97	_at_ 0xc097;
xdata BYTE REG98	_at_ 0xc098;
xdata BYTE REG99	_at_ 0xc099;
xdata BYTE REG9a	_at_ 0xc09a;
xdata BYTE REG9b	_at_ 0xc09b;
xdata BYTE REG9c	_at_ 0xc09c;
xdata BYTE REG9d	_at_ 0xc09d;
xdata BYTE REG9e	_at_ 0xc09e;
xdata BYTE REG9f	_at_ 0xc09f;
xdata BYTE REGa0	_at_ 0xc0a0;
xdata BYTE REGa1	_at_ 0xc0a1;
xdata BYTE REGa2	_at_ 0xc0a2;
xdata BYTE REGa3	_at_ 0xc0a3;
xdata BYTE REGa4	_at_ 0xc0a4;
xdata BYTE REGa5	_at_ 0xc0a5;
xdata BYTE REGa6	_at_ 0xc0a6;
xdata BYTE REGa7	_at_ 0xc0a7;
xdata BYTE REGa8	_at_ 0xc0a8;
xdata BYTE REGa9	_at_ 0xc0a9;
xdata BYTE REGaa	_at_ 0xc0aa;
xdata BYTE REGab	_at_ 0xc0ab;
xdata BYTE REGac	_at_ 0xc0ac;
xdata BYTE REGad	_at_ 0xc0ad;
xdata BYTE REGae	_at_ 0xc0ae;
xdata BYTE REGaf	_at_ 0xc0af;
xdata BYTE REGb0	_at_ 0xc0b0;
xdata BYTE REGb1	_at_ 0xc0b1;
xdata BYTE REGb2	_at_ 0xc0b2;
xdata BYTE REGb3	_at_ 0xc0b3;
xdata BYTE REGb4	_at_ 0xc0b4;
xdata BYTE REGb5	_at_ 0xc0b5;
xdata BYTE REGb6	_at_ 0xc0b6;
xdata BYTE REGb7	_at_ 0xc0b7;
xdata BYTE REGb8	_at_ 0xc0b8;
xdata BYTE REGb9	_at_ 0xc0b9;
xdata BYTE REGba	_at_ 0xc0ba;
xdata BYTE REGbb	_at_ 0xc0bb;
xdata BYTE REGbc	_at_ 0xc0bc;
xdata BYTE REGbd	_at_ 0xc0bd;
xdata BYTE REGbe	_at_ 0xc0be;
xdata BYTE REGcf	_at_ 0xc0cf;
xdata BYTE REGc0	_at_ 0xc0c0;
xdata BYTE REGc1	_at_ 0xc0c1;
xdata BYTE REGc2	_at_ 0xc0c2;
xdata BYTE REGc3	_at_ 0xc0c3;
xdata BYTE REGc4	_at_ 0xc0c4;
xdata BYTE REGc5	_at_ 0xc0c5;
xdata BYTE REGc6	_at_ 0xc0c6;
xdata BYTE REGc7	_at_ 0xc0c7;
xdata BYTE REGc8	_at_ 0xc0c8;
xdata BYTE REGc9	_at_ 0xc0c9;
xdata BYTE REGca	_at_ 0xc0ca;
xdata BYTE REGcb	_at_ 0xc0cb;
xdata BYTE REGcc	_at_ 0xc0cc;
xdata BYTE REGcd	_at_ 0xc0cd;
xdata BYTE REGce	_at_ 0xc0ce;
xdata BYTE REGcf	_at_ 0xc0cf;
xdata BYTE REGd0	_at_ 0xc0d0;
xdata BYTE REGd1	_at_ 0xc0d1;
xdata BYTE REGd2	_at_ 0xc0d2;
xdata BYTE REGd3	_at_ 0xc0d3;
xdata BYTE REGd4	_at_ 0xc0d4;
xdata BYTE REGd5	_at_ 0xc0d5;
xdata BYTE REGd6	_at_ 0xc0d6;
xdata BYTE REGd7	_at_ 0xc0d7;
xdata BYTE REGd8	_at_ 0xc0d8;
xdata BYTE REGd9	_at_ 0xc0d9;
xdata BYTE REGda	_at_ 0xc0da;
xdata BYTE REGdb	_at_ 0xc0db;
xdata BYTE REGdc	_at_ 0xc0dc;
xdata BYTE REGdd	_at_ 0xc0dd;
xdata BYTE REGde	_at_ 0xc0de;
xdata BYTE REGdf	_at_ 0xc0df;
xdata BYTE REGe0	_at_ 0xc0e0;
xdata BYTE REGe1	_at_ 0xc0e1;
xdata BYTE REGe2	_at_ 0xc0e2;
xdata BYTE REGe3	_at_ 0xc0e3;
xdata BYTE REGe4	_at_ 0xc0e4;
xdata BYTE REGe5	_at_ 0xc0e5;
xdata BYTE REGe6	_at_ 0xc0e6;
xdata BYTE REGe7	_at_ 0xc0e7;
xdata BYTE REGe8	_at_ 0xc0e8;
xdata BYTE REGe9	_at_ 0xc0e9;
xdata BYTE REGea	_at_ 0xc0ea;
xdata BYTE REGeb	_at_ 0xc0eb;
xdata BYTE REGec	_at_ 0xc0ec;
xdata BYTE REGed	_at_ 0xc0ed;
xdata BYTE REGee	_at_ 0xc0ee;
xdata BYTE REGef	_at_ 0xc0ef;
xdata BYTE REGf0	_at_ 0xc0f0;
xdata BYTE REGf1	_at_ 0xc0f1;
xdata BYTE REGf2	_at_ 0xc0f2;
xdata BYTE REGf3	_at_ 0xc0f3;
xdata BYTE REGf4	_at_ 0xc0f4;
xdata BYTE REGf5	_at_ 0xc0f5;
xdata BYTE REGf6	_at_ 0xc0f6;
xdata BYTE REGf7	_at_ 0xc0f7;
xdata BYTE REGf8	_at_ 0xc0f8;
xdata BYTE REGf9	_at_ 0xc0f9;
xdata BYTE REGfa	_at_ 0xc0fa;
xdata BYTE REGfb	_at_ 0xc0fb;
xdata BYTE REGfc	_at_ 0xc0fc;
xdata BYTE REGfd	_at_ 0xc0fd;
xdata BYTE REGfe	_at_ 0xc0fe;
xdata BYTE REGff	_at_ 0xc0ff;
#endif	// internal MCU only variables

#endif  //__VARIABLES__

