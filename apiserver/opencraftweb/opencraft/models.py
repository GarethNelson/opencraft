import uuid
from datetime import datetime
from django.db import models


class OpencraftUser(models.Model):
   UserID               = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
   UserUsername         = models.CharField(max_length=30)
   UserEmail            = models.EmailField()
   UserDOB              = models.DateField(default=datetime.now)
   UserEmailVerified    = models.BooleanField(default=False)
   UserEmailVerifyToken = models.UUIDField(default=uuid.uuid4, editable=False)
   UserRegistered       = models.DateTimeField(auto_now_add=True)
   UserPasswordChange   = models.DateTimeField(default=datetime.now)
   UserPasswordHash     = models.CharField(max_length=60,default="")

class OpencraftUserProfile(models.Model):
   ProfileID            = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
   UserID               = models.ForeignKey(OpencraftUser, on_delete=models.CASCADE)
   UserPlayerName       = models.CharField(max_length=16)
   UserSkinURL          = models.URLField()

class OpencraftUserProperty(models.Model):
   UserID               = models.ForeignKey(OpencraftUser, on_delete=models.CASCADE)
   PropertyName         = models.CharField(max_length=30)
   PropertyValue        = models.TextField()

class OpencraftGameSession(models.Model):
   SessionID            = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
   UserID               = models.ForeignKey(OpencraftUser, on_delete=models.CASCADE)
   ProfileID            = models.ForeignKey(OpencraftUserProfile, on_delete=models.CASCADE)
   ClientToken          = models.CharField(max_length=36)
   AccessToken          = models.CharField(max_length=36)
   LastAccess           = models.DateTimeField(auto_now=True)
