#!/usr/bin/perl -w
#
# hgTest.pl: parameterizable test for hg CGI tools.
# See usage for a description of parameters.
#

# Figure out path of executable so we can add perllib to the path.
use FindBin qw($Bin);
use lib "$Bin/perllib";
use TrackDb;
use WebTest;

use LWP::UserAgent;
# use LWP::Debug qw(+);
use HTTP::Cookies;
use HTML::Form;
use Getopt::Long;
use Carp;
use strict;

#
# Default behaviors, changeable by command line args:
#
my $webserv = $ENV{HOST};
$webserv = 'hgwdev' if (! defined $webserv);
my $db      = 'hg10';
my $cookies = 'y';
my $sid     = 'n';
my $rand    = 0;
my $sleep   = 0;
my $verbose = 0;
my $debug   = 0;

# Hard-coded behaviors:
my $basename    = $0;  $basename =~ s@.*/@@;
my $domain      = '.soe.ucsc.edu';
my $cookieFile  = '/tmp/hgTestCookies';
my @hgTsearch = ('Chr7', '20p13', 'chr3:1-1000000', 'D16S3046', 
		 'D22S586;D22S43', 'AA205474',
		 # these are in testplan but not hg10: 'ctg13698', 'AP001670', 
		 'AF083811', 'PRNP', 'pseudogene mRNA', 'homeobox caudal', 
		 'valyl-tRNA', 'zinc finger', 'kruppel zinc finger', 
		 'huntington', 'zahler', 'Evans,J.E.',
		 'chr22:15673184-15700410');
my @hgTsearchBad = ('chr25',
		    # these don't return err: 'Evans JE', 'chr3:1-123456780',
		    '');
my @hgTwidths = (1, 100, 320, 900, 3000);
my @hgTwidthsBad = (-1, 0, 10000, 'a', '+', '\P', '');
my $hgsid = 0;
my $nasty = 0;
if ($nasty) {
    # This breaks at 100:
    push @hgTsearchBad, &bufferAttack(100);
    # I went up to 65536 on this one and it just wouldn't die:
    push @hgTwidthsBad, &bufferAttack(1024);
}
my $tests = 'hgTracks:hgTrackUi:hgGateway:hgc:hgBlat:hgConvCoords:hgText';

#
# usage: Print help message and exit, happy or unhappy.
#
sub usage {
    print STDERR "Usage:
$basename  [-webserv h]  [-db d]  [-cookies y/n]  [-sid y/n]  [-rand N]  [-sleep N]  [-help]  [-v]
    -webserv h:		Use h as the target web server.  Default: $webserv.
			[h can be a colon-separated list.]
    -db d:		Use db as the genome database.  Default: $db.
			[d can be a colon-separated list.]
    -cookies y/n:	Whether to use cookies.  Default: $cookies.
    -sleep N:		Sleep N seconds between queries.  Default: $sleep.
    -sid y/n:		Whether to use session ID.  Default: $sid.  Not implemented!
    -rand N:            Generate N randomized queries.  Default: $rand.  Not implemented!
    -help:		Print this message.
    -verbose:		Print lots of debugging output.
";
    exit(@_);
} # end usage


###########################################################################
#
# Parse & process command line args
#
# GetOptions will put command line args here:
use vars qw/
    $opt_webserv
    $opt_db
    $opt_cookies
    $opt_sid
    $opt_rand
    $opt_sleep
    $opt_help
    $opt_verbose
    /;

my $ok = GetOptions("webserv=s",
		    "db=s",
		    "cookies=s",
		    "sid=s",
		    "rand=i",
		    "sleep=i",
		    "help",
		    "verbose");
&usage(1) if (! $ok);
&usage(0) if ($opt_help);
$webserv  = $opt_webserv if ($opt_webserv);
my @webservs = split(/:/, $webserv);
foreach my $h (@webservs) {
    # cookies don't work if the web server host is given without 
    # a domain;  somewhere along the way, .local is appended if no 
    # domain, e.g. hgwdev-->hgwdev.local.  Anyway, this fixes it:
    $h .= $domain if ($h !~ /\./);
}
$db       = $opt_db if ($opt_db);
my @dbs   = split(/:/, $db);
$cookies  = $opt_cookies if (defined $opt_cookies);
$sid      = $opt_sid if (defined $opt_sid);
$rand     = $opt_rand if (defined $opt_rand);
$sleep    = $opt_sleep if (defined $opt_sleep);
$verbose  = $opt_verbose if (defined $opt_verbose);
$verbose  = 1 if ($debug);

print "
$basename parameters:
---------------------
webservs: @webservs
dbs:      @dbs
cookies?: $cookies
sleep:    $sleep
sid?:     $sid
rand:     $rand

" if ($verbose);

# booleanize $cookies and $sid for convenience
$cookies = ($cookies =~ m/^[yt]/i) ? 1 : 0;
$sid     = ($sid     =~ m/^[yt]/i) ? 1 : 0;


###########################################################################
#
# Initialize helper objects.
#
# Create HTTP UserAgent, init cookies if specified.
my $ua = LWP::UserAgent->new;
if ($cookies) {
    $ua->cookie_jar(HTTP::Cookies->new(file     => $cookieFile,
				       autosave => 1));
    print "\nSet up cookie jar in $cookieFile\n\n" if ($verbose);
}
# Pass the UserAgent handle to our web tester.  
my $webTest = new WebTest($ua, $sleep, $verbose, $debug);
# Make some configs for the webTester.
my $expectFail = {'mustMatch'    => ['(sorry|error|can\'t|Please go back)']};
my $expectPass = {'mustNotMatch' => ['(sorry|error|can\'t)']};
my $expectHgTU = {'mustNotMatch' => ['not found']};
# this isn't used, but I'll leave it here as an example of using mult. pat's:
my $expectHgTr = {'mustNotMatch' => ['(sorry|error|can\'t)'],
		  'mustMatch'    => ['UCSC Genome Browser on .* Freeze'],
	         };

###########################################################################
#
# For each webserv & db, test all the CGI tools on all the tracks....
#
my $totalGood = 0;
my $totalBad  = 0;
foreach my $webserv (@webservs) {
    foreach my $db (@dbs) {
	if ($tests =~ /hgGateway/) {
	    my ($good, $bad) = &hgGateway($webTest, $webserv, $db);
	    $totalGood += $good;  $totalBad += $bad;
	}
	if ($tests =~ /hgTracks/) {
	    my ($good, $bad) = &hgTracks($webTest, $webserv, $db);
	    $totalGood += $good;  $totalBad += $bad;
	}
	if ($tests =~ /hgTrackUi/) {
	    my ($good, $bad) = &hgTrackUi($webTest, $webserv, $db);
	    $totalGood += $good;  $totalBad += $bad;
	}
    } # each db
} # each webserv
print "Summary: $totalGood successful queries, $totalBad failures.\n";

###########################################################################
#
# hgTrackUi query generator.
#
sub hgTrackUi {
    my $webTest = shift;
    my $webserv = shift;
    my $db      = shift;
    confess "too few args" if (! defined $db);
    confess "too many args" if (defined shift);

    my ($good, $bad) = (0, 0);
    my $tdb = new TrackDb($db);
    my @tracks = $tdb->getTrackNames();
    $tdb->DESTROY(); # why wait for perl...
    foreach my $track (@tracks) {
	my $page  = "http://$webserv/cgi-bin/hgTrackUi";
	my $query = "db=$db&g=$track&c=stub";
	$webTest->configure($expectHgTU);
	my $ok = $webTest->checkPage($page, $query);
	$ok ? $good++ : $bad++;
    } # each trackName
    return($good, $bad);
} # end hgTrackUi


###########################################################################
#
# hgGateway query generator.
#
sub hgGateway {
    my $webTest = shift;
    my $webserv = shift;
    my $db      = shift;
    confess "too few args" if (! defined $db);
    confess "too many args" if (defined shift);

    my $page  = "http://$webserv/cgi-bin/hgGateway";
    my $query = "db=$db";
    $webTest->configure($expectPass);
    my $ok = $webTest->checkPage($page, $query);
    $ok ? return(1, 0) : return(0, 1);
} # end hgGateway



###########################################################################
#
# hgTracks
#
sub hgTracks {
    my $webTest = shift;
    my $webserv = shift;
    my $db      = shift;
    confess "too few args" if (! defined $db);
    confess "too many args" if (defined shift);

    my ($good, $bad) = (0, 0);

    my $page  = "http://$webserv/cgi-bin/hgTracks";

    #
    # If using cookies, do a cartReset here so we get consistent 
    # results from run to run!  (All the button-clicking below 
    # was causing some runs to start with hideAll...)
    #
    &cartReset($webserv) if ($cookies);

    #
    # some bounds-checking on position and width inputs:
    #
    # expect to find matches for all these items:
    $webTest->configure($expectPass);
    foreach my $search (@hgTsearch) {
	my $query = "db=$db&position=$search&pix=100";
	$query .= "&hgsid=$hgsid" if ($sid =~ m/^[yt]/i);
	my $ok = $webTest->checkPage($page, $query);
	$ok ? $good++ : $bad++;
    }
    # but not these items:
    $webTest->configure($expectFail);
    foreach my $search (@hgTsearchBad) {
	my $query = "db=$db&position=$search&pix=100";
	$query .= "&hgsid=$hgsid" if ($sid =~ m/^[yt]/i);
	my $ok = $webTest->checkPage($page, $query);
	$ok ? $good++ : $bad++;
    }
    # expect good and bad widths to return OK:
    $webTest->configure($expectPass);
    my $search = 'chr1:1-10';
    foreach my $width (@hgTwidths, @hgTwidthsBad) {
	my $query = "db=$db&position=$search&pix=$width";
	$query .= "&hgsid=$hgsid" if ($sid =~ m/^[yt]/i);
	my $ok = $webTest->checkPage($page, $query);
	$ok ? $good++ : $bad++;
    }

    #
    # Now feed in some nice simple inputs, parse the returned page 
    # into a form & links, and play with the form.
    #
    my $query = "db=$db&position=chr22:15550662-15822931&pix=100";
    my $html  = $webTest->getPage($page, $query);
    $webTest->configure($expectPass);
    if (defined $html) {
	if (open(OUT, ">/tmp/vanilla.out")) {
	    print OUT $html;
	    close(OUT);
	}
	# click every submit button on this page.
	my $form  = HTML::Form->parse($html, $page);
	if (! defined $form) {
	    $bad++;
	    print "Couldn't parse $page?$query text!\n";
	    return ($good, $bad);
	}
	my @inputs = $form->inputs();
	print "hgTracks: Checking " . scalar(@inputs) . " form inputs.\n" if ($verbose);
	foreach my $i (@inputs) {
	    print $i->type() . ' -> ' . $i->name()  . "\n" if ($debug);
	    if ($i->type() eq 'submit') {
		my $req = $form->click($i->name());
		my $ok = $webTest->checkRequest($req);
		$ok ? $good++ : $bad++;
	    }
	}
	# follow every link on this page.
	my @links = &getLinks($html);
	print "hgTracks: Checking " . scalar(@links) . " links.\n" if ($verbose);
	foreach my $l (@links) {
	    # hacky way to split up the link, but it'll have to do for now.
	    my ($page, $query);
	    if ($l =~ /^([\w\.\:\/\-\_]+)\?(.+)$/) {
		$page = $1;
		$query = $2;
	    } else {
		$page = $l;
	    }
	    if ($page !~ /^http:/) {
		# hacky way to fix relative URL or ignore link.
		next if ($page !~ s/^.*cgi\-bin/http\:\/\/$webserv\/cgi\-bin/);
	    }
	    my $ok = $webTest->checkPage($page, $query);
	    $ok ? $good++ : $bad++;
	}

	# do another cartReset before starting this test:
	&cartReset($webserv) if ($cookies);
	#
	# Jim's recommended sequece:
	# - click hideAll.
	# - then, starting from that page:
	#   - for each track:
	#     - select dense, then click submit
	#     - select full,  then click submit
	# click every submit button on this page.
	my $i = $form->find_input('hgt.hideAll');
	if (! defined $i) {
	    print "DOH! Couldn't find hgt.hideAll in this:\n$html\n";
	    return ($good, $bad);
	}
	my $req = $form->click($i->name());
	my $newhtml = $webTest->getRequest($req);
	if (defined $newhtml) {
	    if (open(OUT, ">/tmp/hideall.out")) {
		print OUT $newhtml;
		close(OUT);
	    }
	    my $newform  = HTML::Form->parse($newhtml, $page);
	    my @newinputs = $newform->inputs();
	    foreach my $n (@newinputs) {
		if ($n->type() eq 'option') {
		    # dense check
		    if (grep /dense/, $n->possible_values()) {
			$newform->value($n->name(), 'dense');
			my $req = $newform->click('submit');
			my $ok = $webTest->checkRequest($req);
			$ok ? $good++ : $bad++;
		    }
		    # full check
		    if (grep /full/, $n->possible_values()) {
			$newform->value($n->name(), 'full');
			$req = $newform->click('submit');
			$ok = $webTest->checkRequest($req);
			$ok ? $good++ : $bad++;
		    }
		}
	    }
	} else {
	    print "Couldn't get html for vanilla+hideAll page?!\n" if ($verbose);
	    $bad++;
	}
    } else {
	print "Couldn't get html for vanilla starting page?!\n" if ($verbose);
	$bad++;
    }

    return($good, $bad);
} # end hgTracks



###########################################################################
#
# hgBlat
#


###########################################################################
#
# hgConvCoords
#


###########################################################################
#
# hgText
#



#
# bufferAttack: make a string of the specified length.
#
sub bufferAttack {
    my $length = shift;
    confess "too few args" if (! defined $length);
    confess "too many args" if (defined shift);

    my $str = 'b';
    while (length($str) < $length) {
	$str .= $str;
    }
    $str = substr($str, 0, $length);
    return($str);
}


#
# getLinks: return a list of all HREF's in the input text:
#
sub getLinks {
    my $html = shift;
    confess "too few args" if (! defined $html);
    confess "too many args" if (defined shift);
    my $copy = $html;
    my @links = ();
    while ($copy =~ s/[hH][rR][eE][fF]\s*=\s*[\"\']([^\"\']+)[\"\']//) {
	push @links, $1;
    }
    return @links;
} # end getLinks


#
# cartReset: run cgi-bin/cartReset to clean the user-interface slate.
#
sub cartReset {
    my $webserv = shift;
    confess "too few args" if (! defined $webserv);
    confess "too many args" if (defined shift);
    my $p = "http://$webserv/cgi-bin/cartReset";
    $webTest->getPage($p);
}

