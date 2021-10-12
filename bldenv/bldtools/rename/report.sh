#! /bin/ksh
#
# @(#)33	1.10  src/bldenv/bldtools/rename/report.sh, bldtools, bos412, GOLDA411a 8/20/93 17:31:03
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: report
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# General report utility.
#
. bldkshconst
. bldinitfunc

chksetbecome
chksetfamily

test ${BLDTMP:=$(bldtmppath)}

typeset -u area
typeset -r tmp_file=${BLDTMP}/report_$(uname -n)_$$

area="$(Report -view user -where "login='${CMVC_BECOME}'" -raw | cut -f3 -d'|')"

print
print "Level|Release|Type|OwnerLogin|OwnerName|OwnerArea|DateCreated|DateCommitted|LastUpdate|State" > ${tmp_file}
case $1 in
  "")
    Report -view LevelView \
           -where "upper(userArea) like '%${area}%' and 
                   commitDate is NULL and 
                   type != 'other' 
                   order by releaseName,name" \
           -raw \
    >> ${tmp_file}
  ;;
  all)
    Report -view LevelView \
           -where "upper(userArea) like '%${area}%'
                   order by releaseName,name" \
           -raw \
    >>  ${tmp_file}
  ;;
  def)
    Report -view LevelView \
           -long \
           -where "upper(userArea) like '%${area}%' and
                   name like '$2%' and
                   type = 'production'
                   order by releaseName,name" \
    >  ${tmp_file}.long
    egrep 'p    0|a    0|d    0'  ${tmp_file}.long | wc -l
    rm -f ${tmp_file} ${tmp_file}.long
    exit
  ;;
  other)
    Report -view LevelView \
           -where "upper(userArea) like '%${area}%' and
                   commitDate is NULL and
                   type = 'other'
                   order by releaseName,name" \
           -raw \
    >>  ${tmp_file}
  ;;
  pib)
    Report -view LevelView \
          -where "userLogin = 'pib' and 
                  commitDate is NULL and 
                  type != 'other'
                  order by releaseName,name" \
          -raw \
    >>  ${tmp_file}
  ;;
  selfix|areabld)
    typeset -R5 apars
    typeset -L17 dateassigned
    typeset defect
    typeset -R5 defects
    typeset -L15 level
    typeset prefix
    typeset -L12 release
    typeset -R5 total_apars=0
    typeset -R5 total_defects=0
    typeset -L11 type
    typeset rptid=$1

    print "     Level        Release       Type       DateAssigned    \c"
    print "Defects Apars"
    Report -view LevelView -raw \
           -where "userLogin = '$rptid' and
                   commitDate is NULL and
                   type != 'other' 
                   order by releaseName,name" \
       | cut -d'|' -f1,2,3,9 | sed -e "s/|/ /g" |&
    while read -p level release type dateassigned
    do
       (( defects = 0 ))
       (( apars = 0 ))
       Report -view levelMemberView -raw \
              -where "levelName='`print ${level}`' and 
                      releaseName='`print ${release}`'" \
          | cut -d'|' -f3,8 | sed -e "s/|/ /g" > ${tmp_file}
       while read defect prefix
       do
          (( defects = ${defects}+1 ))
          if [[ "${prefix}" = "a" || "${prefix}" = "xa" ]]
          then
             (( apars = ${apars}+1 ))
          fi
       done < ${tmp_file}
       print "${level} ${release} ${type} ${dateassigned}  ${defects}  ${apars}"
       (( total_defects = ${total_defects}+${defects} ))
       (( total_apars = ${total_apars}+${apars} ))
    done
    print "${level} ${release} ${type}            Total:  \c"
    print "${total_defects}  ${total_apars}"
    rm -f  ${tmp_file}
    print
    exit 0
    ;;
  selfix_defects)
    typeset -L63 abstract
    typeset -R3 age=
    typeset -L10 compName=
    typeset -L10 defect_state=
    typeset -R6 name=
    typeset -L8 ownerLogin=
    typeset -R3 severity=
    typeset -L10 state=
    typeset -L10 track=

    print "\nReport run on `date`.\n"
    print "defect track      compName   state      ownerLog sev age abstract\c"
    print "\n------ ---------- ---------- ---------- -------- --- ---\c"
    print " --------------------------------------------------------------"
    rm -f ${tmp_file}
    IFS="|"
    Report -view defectview \
           -where "state not in ('canceled','closed') and \
                   compname in ('bldtools','bldprocess','bosbuild', \
                                'pkgtools','package','retain')" \
           -raw \
       |&
    
    while read -p dummy name compName dummy ownerLogin defect_state dummy \
                  severity abstract age dummy dummy dummy dummy dummy \
                  dummy dummy dummy dummy dummy dummy dummy dummy dummy \
                  dummy dummy dummy dummy dummy 
    do
       track_count=0
       Report -view trackview \
              -where "defectname = '${name}'" \
              -raw \
       | cut -d\| -f1,4 \
       | while read track state
         do
            (( track_count = ${track_count}+1 ))
            if [ "${state}" != "commit    " -a \
                 "${state}" != "complete  " ]
            then
               print ${name} ${track} ${compName} ${state} ${ownerLogin} \
                     ${severity} ${age} ${abstract} >> ${tmp_file}
            fi
         done
       # If defect has no tracks, still need to report on it.
       if [ ${track_count} -eq 0 ]
       then
          track=""
          print ${name} ${track} ${compName} ${defect_state} ${ownerLogin} \
                ${severity} ${age} ${abstract} >> ${tmp_file}
       fi
    done
    sort -b +f0.29 -f0.38 +f0.18 -f0.28 +f0.0 -f0.6 ${tmp_file}
    rm -f ${tmp_file}
    exit 0
    ;;
  unf)
    Report -view LevelView \
           -raw \
           -where "upper(userArea) like '%${area}%' and
                   type != 'other' and
                   state is NULL or
                   state='clean'
                   order by releaseName,name" \
    >>  ${tmp_file}
  ;;
  *)
    Report -view LevelView \
           -raw \
           -where "upper(userArea) like '%${area}%' and
                   type != 'other'
                   order by releaseName,name" \
    | fgrep $1 \
    >>  ${tmp_file}
  ;;
esac
awk '{ FS="|";
       fm="%-15.15s %-12.12s %-11.11s %-17.17s %+5.5s\n";
       printf fm,$1,$2,$3,$9,$10 }'  ${tmp_file}
print
rm -f  ${tmp_file}
