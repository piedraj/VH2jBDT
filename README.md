# VH2j Boosted Decision Tree

The goal of this multivariate analysis effort is to better separate VH2j from ggH. We have chosen the Boosted Decision Trees (BDT) method as classifier. This code is being developed in `CMSSW_9_4_9`. Downloading the code is easy.

    git clone https://github.com/piedraj/VH2jBDT

This first part has to do with the training and testing of the BDT. The following macro does the job with a given set of variables and BDT configuration. This is the place to play with both the input variables and also the BDT configuration.

    root -l -b -q VH2j_TMVAClassification.C\(\"BDT\"\)

To read the output of the BDT the macro `VH2j_TMVAReader.C` has to be modified, in such a way that it contains exactly the same variables as in `VH2j_TMVAClassification.C`. Not only that, the variables have to follow the precise same order, and the names have to be the same as in the classifier. Then, to apply the reader on latino trees the following lines must be added in `variables.py`.

    variables['bdt'] = {
         'name': 'VH2j_TMVAReader(Entry$)',
         'range' : (20,-1.,1.),
         'xaxis' : 'BDT discriminant VH2j',
         'fold' : 3,
         'linesToAdd' : ['.L /afs/cern.ch/user/p/piedra/work/VH2jBDT/VH2j_TMVAReader.C+']
    }

# VH2j MultiClass

    root -l -b -q VH2j_TMVAMultiClass.C+

    root -l VH2j_TMVAMultiClassGui.C

