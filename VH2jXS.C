void VH2jXS()
{
  float br_w_en = 0.1071;  // W   -> e   nu
  float br_w_mn = 0.1063;  // W   -> mu  nu
  float br_w_tn = 0.1138;  // W   -> tau nu
  float br_t_mn = 0.1739;  // tau -> mu  nu
  float br_t_en = 0.1782;  // tau -> e   nu
  float br_w_qq = 0.6741;  // W   -> q   q
  float br_z_qq = 0.6911;  // Z   -> q   q
  float br_h_ww = 0.2137;  // H   -> W   W

  float xs_wh = 1.358;  // pb
  float xs_zh = 0.880;  // pb

  float xs_vh_qq = (br_w_qq * xs_wh) + (br_z_qq * xs_zh);

  float br_ww_em = (2. * br_w_en * br_w_mn) + (2. * br_w_en * br_w_tn * br_t_mn) + (2. * br_w_mn * br_w_tn * br_t_en) + (2. * br_w_tn * br_t_en * br_w_tn * br_t_mn);  // WW -> em

  float luminosity = 41500;  // pb-1

  float xs_vh2j_qqem = xs_vh_qq * br_h_ww * br_ww_em; 

  float n_vh2j_qqem = xs_vh2j_qqem * luminosity;

  printf("\n");
  printf("                  luminosity = %.0f pb-1\n", luminosity);
  printf("                BR(WW -> em) = %f\n", br_ww_em);
  printf("         xs(pp -> VH -> qqH) = %f pb\n", xs_vh_qq);
  printf(" xs(pp -> VH -> VWW -> qqem) = %f pb\n", xs_vh2j_qqem);
  printf("  N(pp -> VH -> VWW -> qqem) = %.0f\n", n_vh2j_qqem);
  printf("\n");
}
