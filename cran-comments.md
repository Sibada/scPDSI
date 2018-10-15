## Resubmission
This is a resubmission. In this version we have:

Oct. 19th, 2018:

* Revised the DOI and url of the references in the descrpition field in the DESCRIPTION file.

Many thanks again for your review of my package.

Best regards.


Oct. 18th, 2018:

* Revised the DESCRIPTION file, including: remove the "(scPDSI)" from title field and remove the space of "C++" in the Description field.

* Add the url and doi of the references to the Description field of the DESCRIPTION file. Note that the paper Palmer (1965) has no doi because for its age, and I could only find a relatively stable url for the pdf of this paper of NOAA's website; the challenge of adding the doi of Wells et al. (2004) is that its doi also contains angle bracket (10.1175/1520-0442(2004)017<2335:ASPDSI>2.0.CO;2) and I afraid that it would cause identification error of the doi. Now I try to add it, and if it do not works please infrom.

* The authors of the C++ source code of scPDSI and the relevant paper (Wells et al., 2004), Prof. Steve Goddard, Dr. Nathan Wells, and Dr. Mike Hayes, are willing to be the "contributer" roles in the author list.


Oct. 6th, 2018:

* Revised the format of references in the DESCRIPTION file.

* As for the copyright of the source C++ codes, I have required prof. Steve Goddard (one of the author of those source codes of scPDSI and the corresponding author of the relevant paper (Wells et al., 2004)). He replied that they do not hold the copyright of the codes and permit for freely secondary development. We also did not find any copyright statements in those codes as we did not do any change in the header comments of the codes. Therefore, we only cited the reference of their corresponding publication i.e. Wells et al. (2004) in the DESCRIPTION file and the documentation as prof. Goddard suggested.

This is the reply mail of prof.
   Goddard:
"I think it would be great for the community if you created an R package to calculate the scPDSI. I don¡¯t believe we put any copyright on the code. Please feel free to adapt the code for secondary development. and add a copyright for your contributions. It would be appreciated if you also cited our journal paper in the comments. (We may have already done that. I don¡¯t recall.)

Good luck!

Steve
goddard@cse.unl.edu".

My request mail is:
"Dear Professor Goddard
     I am a BSc in Sun Yat-sen university and I have hear your contribution for the sc-PDSI. I think that an R package to calculate scPDSI would largely help the scientists who use R to study drought and climate change. However, At present I have not found a package about PDSI.
     Therefore I plant to develop an R package specified for sc-PDSI and release it to CRAN. It would be most convinient to adapt the cpp codes for scPDSI calculating circulating online to the framework of R package. But I could find any copyright statement in this cpp file. Therefore could I ask you for whether those codes have copyrights? And are those codes open source? Could someone adapt it for secondary development? If true, what should the developers do to protect the intellectual property of you and other contributers?
     Since I want to write the R package as an open source software and release to public, those would be important. I would really appreciate for your response.

Best regards".

Many thanks for your review.

## Test environments
* local OS X install, R 3.2.3
* ubuntu 12.04 (on travis-ci), R 3.2.3
* win-builder (devel and release)

## R CMD check results

0 errors | 0 warnings | 1 note

* This is a new release.

## Reverse dependencies

This is a new release, so there are no reverse dependencies.

